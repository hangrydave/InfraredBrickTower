#include "pch.h"
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <winusb.h>
#include <usb.h>

#include "device.h"

#include "WinUsbTowerInterface.h"
#include <stdio.h>
#include <winusb.h>

// useful documentation here: https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/using-winusb-api-to-communicate-with-a-usb-device#step-3-send-control-transfer-to-the-default-endpoint

// NQC makefile says that legousbtower.c (Linux kernel module) has it as 200
#define READ_TIMEOUT 300
#define WRITE_TIMEOUT 9999999 // idek at this point 

#define DEBUG 0

BOOL OpenWinUsbTowerInterface(HostTowerCommInterface*& towerInterface)
{
	DEVICE_DATA           deviceData;
	HRESULT               hr;
	USB_DEVICE_DESCRIPTOR deviceDesc;
	BOOL                  bResult;
	BOOL                  noDevice;
	ULONG                 lengthReceived;

	//
	// Find a device connected to the system that has WinUSB installed using our
	// INF
	//
	hr = OpenDevice(&deviceData, &noDevice);

	if (FAILED(hr))
	{
		if (noDevice)
		{
			printf("Device not connected or driver not installed\n");
		}
		else
		{
			printf("Failed looking for device, HRESULT 0x%x\n", hr);
		}

		return FALSE;
	}

	//
	// Get device descriptor
	//
	bResult = WinUsb_GetDescriptor(deviceData.WinusbHandle,
		USB_DEVICE_DESCRIPTOR_TYPE,
		0,
		0,
		(PBYTE)&deviceDesc,
		sizeof(deviceDesc),
		&lengthReceived);

	if (FALSE == bResult || lengthReceived != sizeof(deviceDesc))
	{
		printf("Error among LastError %d or lengthReceived %d\n",
			FALSE == bResult ? GetLastError() : 0,
			lengthReceived);
		CloseDevice(&deviceData);
		return FALSE;
	}

	USB_CONFIGURATION_DESCRIPTOR configDescriptor;
	ULONG cdLenReceived;
	WinUsb_GetDescriptor(
		deviceData.WinusbHandle,
		USB_CONFIGURATION_DESCRIPTOR_TYPE,
		0,
		0,
		(PBYTE)&configDescriptor,
		sizeof(configDescriptor),
		&cdLenReceived
	);

	ULONG timeout = WRITE_TIMEOUT;
	PULONG timeoutPointer = &timeout;
	BOOL policySetResult = WinUsb_SetPipePolicy(
		deviceData.WinusbHandle,
		TOWER_WRITE_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutPointer
	);

	if (!policySetResult)
	{
		printf("WinUSB set write pipe policy error");
#if DEBUG == 1
		__debugbreak();
#endif
		return FALSE;
	}

	timeout = READ_TIMEOUT;
	timeoutPointer = &timeout;
	policySetResult = WinUsb_SetPipePolicy(
		deviceData.WinusbHandle,
		TOWER_READ_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutPointer
	);

	if (!policySetResult)
	{
		printf("WinUSB set read pipe policy error");
#if DEBUG == 1
		__debugbreak();
#endif
		return FALSE;
	}

	towerInterface = new WinUsbTowerInterface(
		deviceData.HandlesOpen,
		deviceData.WinusbHandle,
		deviceData.DeviceHandle,
		deviceData.DevicePath);

	return TRUE;
}

WinUsbTowerInterface::WinUsbTowerInterface(
	BOOL handlesOpen,
	PVOID winUsbHandle,
	HANDLE deviceHandle,
	PTCHAR devicePath)
{
	this->handlesOpen = handlesOpen;
	this->winUsbHandle = winUsbHandle;
	this->deviceHandle = deviceHandle;
	this->devicePath = devicePath;
	this->timeoutCounter = MAX_TIMEOUT_COUNT;
}

WinUsbTowerInterface::~WinUsbTowerInterface()
{
}

bool WinUsbTowerInterface::ControlTransfer(
	BYTE request,
	WORD value,
	WORD index,
	USHORT bufferLength,
	BYTE* buffer,
	ULONG& lengthTransferred) const
{
	WINUSB_SETUP_PACKET setupPacket;
	setupPacket.RequestType = 0xc0; // table 9-2 in https://fabiensanglard.net/usbcheat/usb1.1.pdf
	setupPacket.Request = request;
	setupPacket.Value = value;
	setupPacket.Index = index;
	setupPacket.Length = bufferLength;

	BOOL success = WinUsb_ControlTransfer(
		this->winUsbHandle,
		setupPacket,
		buffer,
		bufferLength,
		&lengthTransferred,
		NULL
	);

#if DEBUG == 1
	if (!success)
		__debugbreak();
#endif
	PrintErrorIfAny("WinUSB control transfer error");

	return success;
}

bool WinUsbTowerInterface::Write(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthWritten) const
{
	BOOL success = WinUsb_WritePipe(
		this->winUsbHandle,
		TOWER_WRITE_PIPE_ID,
		buffer,
		bufferLength,
		&lengthWritten,
		NULL
	);

	PrintErrorIfAny("WinUSB write error");
#if DEBUG == 1

	if (!success)
		__debugbreak();
#endif

	return success;
}

bool WinUsbTowerInterface::Read(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthRead)
{
	BOOL success = WinUsb_ReadPipe(
		this->winUsbHandle,
		TOWER_READ_PIPE_ID,
		buffer,
		bufferLength,
		&lengthRead,
		NULL
	);

	if (GetLastError() == ERROR_SEM_TIMEOUT)
	{
		// Don't bother printing an error for every read, it's no biggie.
		// The console gets spammed with these due to the read flush before writing.
		
		if (timeoutCounter++ == 3)
		{
			printf("Read timed out %d times, is your RCX on? If it is, try pulling a battery out and putting it back in.\n", MAX_TIMEOUT_COUNT);
		}

#if DEBUG == 1
		if (!success)
			__debugbreak();
#endif
	}
	else
	{
		PrintErrorIfAny("WinUSB read error");
	}

	return success;	
}

bool WinUsbTowerInterface::Flush() const
{
	WinUsb_FlushPipe(this->winUsbHandle, TOWER_WRITE_PIPE_ID);
	WinUsb_FlushPipe(this->winUsbHandle, TOWER_READ_PIPE_ID);
	return TRUE;
}

VOID WinUsbTowerInterface::PrintErrorIfAny(const char* caller) const
{
	DWORD error = GetLastError();
	switch (error)
	{
	case ERROR_INVALID_HANDLE:
		printf("%s: invalid WinUsb handle provided\n", caller);
		break;
	case ERROR_INVALID_PARAMETER:
		printf("%s: invalid parameter\n", caller);
		break;
	case ERROR_IO_PENDING:
		printf("%s: overlapped I\\O operation in progress, operation executing in background\n", caller);
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
		printf("%s: not enough memory\n", caller);
		break;
	case ERROR_SEM_TIMEOUT:
		printf("%s: timeout\n", caller);
		break;
	}
}

bool WinUsbTowerInterface::Close() const
{
	return true;
}
