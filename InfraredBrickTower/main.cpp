#include "pch.h"

#include "USBTowerController.h"
#include "WinUsbTowerInterface.h"
#include "VLLCommands.h"

#include <assert.h>
#include <stdio.h>
#include <iostream>

//BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PUCHAR pipeid);

VOID TestTower(USBTowerController* controller);
BOOL SendPacket(PUCHAR buffer, ULONG bufferLength, PUCHAR replyBuffer, ULONG expectedReplyLength, const DEVICE_DATA& deviceData);

VOID BeepRCXAndMicroScout(USBTowerController* controller);

LONG __cdecl
_tmain(
	LONG     Argc,
	LPTSTR* Argv
)
/*++

Routine description:

	Sample program that communicates with a USB device using WinUSB

--*/
{
	DEVICE_DATA           deviceData;
	HRESULT               hr;
	USB_DEVICE_DESCRIPTOR deviceDesc;
	BOOL                  bResult;
	BOOL                  noDevice;
	ULONG                 lengthReceived;

	UNREFERENCED_PARAMETER(Argc);
	UNREFERENCED_PARAMETER(Argv);

	//
	// Find a device connected to the system that has WinUSB installed using our
	// INF
	//
	hr = OpenDevice(&deviceData, &noDevice);

	if (FAILED(hr)) {

		if (noDevice) {

			wprintf(L"Device not connected or driver not installed\n");

		}
		else {

			wprintf(L"Failed looking for device, HRESULT 0x%x\n", hr);
		}

		return 0;
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

	if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

		wprintf(L"Error among LastError %d or lengthReceived %d\n",
			FALSE == bResult ? GetLastError() : 0,
			lengthReceived);
		CloseDevice(&deviceData);
		return 0;
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

	WinUsbTowerInterface* usbTowerInterface = new WinUsbTowerInterface(&deviceData.WinusbHandle);
	USBTowerController* controller = new USBTowerController(usbTowerInterface);

	BeepRCXAndMicroScout(controller);

	delete controller;
	delete usbTowerInterface;

	CloseDevice(&deviceData);

	system("pause");
	return 0;
}

BOOL SendPacket(
	PUCHAR buffer,
	ULONG bufferLength,
	PUCHAR replyBuffer,
	ULONG expectedReplyLength,
	const DEVICE_DATA& deviceData)
{
	BOOL write = FALSE;
	BOOL read = FALSE;
	while (!write || !read)
	{
		ULONG dataTransferred;
		write = WinUsb_WritePipe(
			deviceData.WinusbHandle,
			2,
			buffer,
			bufferLength,
			&dataTransferred,
			NULL
		);

		ULONG dataReceived;
		read = WinUsb_ReadPipe(
			deviceData.WinusbHandle,
			129,
			replyBuffer,
			expectedReplyLength,
			&dataReceived,
			NULL
		);
	}

	return write;
}

VOID TestTower(USBTowerController* controller)
{
	controller->SetIndicatorLEDMode(TowerIndicatorLEDMode::HOST_SOFTWARE_CONTROLLED);
	controller->SetLEDColor(TowerLED::VLL, TowerLEDColor::DEFAULT);
	controller->SetLEDColor(TowerLED::VLL, TowerLEDColor::OFF);

	INT len;
	CHAR* buffer = 0;

	controller->GetCopyright(buffer, len);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}

	controller->GetCredits(buffer, len);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}
}

VOID BeepRCXAndMicroScout(USBTowerController* controller)
{
	/* MicroScout */
	controller->SetMode(TowerMode::VLL);
	VLL_Beep1Immediate(controller);

	/* RCX */
	controller->SetMode(TowerMode::IR);

	UCHAR replyBuffer[10];
	ULONG replyLen = 10;
	BYTE replyByte;

	ULONG bytesWritten = 0;
	ULONG bytesRead = 0;

	UCHAR ping[] = { 0x55, 0xff, 0x00, 0x10, 0xef, 0x10, 0xef };
	ULONG pingLen = 7;
	controller->WriteData(ping, pingLen, bytesWritten);
	controller->ReadData(replyBuffer, replyLen, bytesRead);
	replyByte = *(replyBuffer + 3) & 0xf7;
	assert(replyByte == 0xE7);

	UCHAR stop[] = { 0x55, 0xff, 0x00, 0x50, 0xaf, 0x50, 0xaf };
	ULONG stopLen = 7;
	controller->WriteData(stop, stopLen, bytesWritten);
	controller->ReadData(replyBuffer, replyLen, bytesRead);
	replyByte = *(replyBuffer + 3) & 0xf7;
	assert(replyByte == 0xA7);

	UCHAR beep[] = { 0x55, 0xff, 0x00, 0x51, 0xae, 0x05, 0xfa, 0x56, 0xa9 };
	ULONG beepLen = 9;
	// expects 0xA6
	controller->WriteData(beep, beepLen, bytesWritten);
	controller->ReadData(replyBuffer, replyLen, bytesRead);
	replyByte = *(replyBuffer + 3) & 0xf7;
	assert(replyByte == 0xA6);
}

// useful documentation here: https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/using-winusb-api-to-communicate-with-a-usb-device#step-3-send-control-transfer-to-the-default-endpoint

//BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PUCHAR pipeID)
//{
//	if (hDeviceHandle == INVALID_HANDLE_VALUE)
//	{
//		return FALSE;
//	}
//
//	BOOL bResult = TRUE;
//
//	USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
//	ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));
//
//	WINUSB_PIPE_INFORMATION  Pipe;
//	ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));
//
//	bResult = WinUsb_QueryInterfaceSettings(hDeviceHandle, 0, &InterfaceDescriptor);
//
//	if (bResult)
//	{
//		for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
//		{
//			bResult = WinUsb_QueryPipe(hDeviceHandle, 0, (UCHAR)index, &Pipe);
//
//			if (bResult)
//			{
//				if (Pipe.PipeType == UsbdPipeTypeInterrupt)
//				{
//					printf("Endpoint index: %d Pipe type: Interrupt Pipe ID: %d.\n", index, Pipe.PipeId);
//					*pipeID = Pipe.PipeId;
//				}
//			}
//			else
//			{
//				continue;
//			}
//		}
//	}
//	return bResult;
//}

