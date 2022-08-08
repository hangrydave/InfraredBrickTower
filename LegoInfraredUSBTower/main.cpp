#include "pch.h"

#include "USBTowerController.h"

#include <stdio.h>
#include <iostream>

//BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, PUCHAR pipeid);

BOOL SendPacket(PUCHAR buffer, ULONG bufferLength, PUCHAR replyBuffer, ULONG expectedReplyLength, const DEVICE_DATA& deviceData);

VOID Beep(const DEVICE_DATA& deviceData);

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

	ULONG timeoutBuffer[1] = { 1000 };
	BOOL policySetResult = WinUsb_SetPipePolicy(
		deviceData.WinusbHandle,
		2,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutBuffer
	);
	if (!policySetResult)
	{
		printf("Failed to set timeout for pipe 2 (write)");
		return 1;
	}

	policySetResult = WinUsb_SetPipePolicy(
		deviceData.WinusbHandle,
		129,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutBuffer
	);
	if (!policySetResult)
	{
		printf("Failed to set timeout for pipe 129 (read)");
		return 1;
	}

	USBTowerController* towerController = new USBTowerController(&deviceData.WinusbHandle);

	towerController->SetIndicatorLEDMode(TowerIndicatorLEDMode::HOST_SOFTWARE_CONTROLLED);
	towerController->SetLEDColor(TowerLED::VLL, TowerLEDColor::DEFAULT);
	towerController->SetLEDColor(TowerLED::VLL, TowerLEDColor::OFF);

	INT len;
	CHAR* buffer = 0;

	towerController->GetCopyright(buffer, len);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}

	towerController->GetCredits(buffer, len);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}

	// from this:

	/*SendControlPacket(LTW_REQ_SET_PARM, LTW_PARM_RANGE, LTW_RANGE_MEDIUM, deviceData);
	SendControlPacket(LTW_REQ_SET_PARM, LTW_PARM_MODE, LTW_MODE_IR, deviceData);
	SendControlPacket(LTW_REQ_SET_TX_SPEED, SPEED_COMM_BAUD_2400, deviceData);
	SendControlPacket(LTW_REQ_SET_RX_SPEED, SPEED_COMM_BAUD_2400, deviceData);
	SendControlPacket(LTW_REQ_SET_TX_CARRIER_FREQUENCY, 0x0026, deviceData);*/

	// to this:

	towerController->SetRange(TowerRange::MEDIUM);
	towerController->SetMode(TowerMode::IR);
	towerController->SetTransmissionSpeed(TowerCommSpeed::COMM_BAUD_2400);
	towerController->SetReceivingSpeed(TowerCommSpeed::COMM_BAUD_2400);

	delete towerController;

	Beep(deviceData);

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
		printf("%d, %d\n", write, read);
	}

	return write;
}

VOID Beep(const DEVICE_DATA& deviceData)
{
	UCHAR replyBuffer[1];
	ULONG replyLen = 1;

	UCHAR ping[] = { 0x55, 0xff, 0x00, 0x10, 0xef, 0x10, 0xef };
	ULONG pingLen = 7;
	SendPacket(ping, pingLen, replyBuffer, replyLen, deviceData);

	UCHAR stop[] = { 0x55, 0xff, 0x00, 0x50, 0xaf, 0x50, 0xaf };
	ULONG stopLen = 7;
	SendPacket(stop, stopLen, replyBuffer, replyLen, deviceData);

	UCHAR beep[] = { 0x55, 0xff, 0x00, 0x51, 0xae, 0x05, 0xfa, 0x56, 0xa9 };
	ULONG beepLen = 9;
	SendPacket(beep, beepLen, replyBuffer, replyLen, deviceData);
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
