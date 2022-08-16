#include "pch.h"
#include "WinUsbTowerInterface.h"
#include <stdio.h>

WinUsbTowerInterface::WinUsbTowerInterface(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;

	ULONG timeout = 1000;
	PULONG timeoutPointer = &timeout;

	BOOL policySetResult = WinUsb_SetPipePolicy(
		*handle,
		TOWER_WRITE_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutPointer
	);

	PrintErrorIfAny("WinUSB set read pipe policy error");
#if DEBUG == 1
	if (!policySetResult)
		__debugbreak();
#endif

	policySetResult = WinUsb_SetPipePolicy(
		*handle,
		TOWER_READ_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutPointer
	);

	PrintErrorIfAny("WinUSB set read pipe policy error");
#if DEBUG == 1
	if (!policySetResult)
		__debugbreak();
#endif
}

BOOL WinUsbTowerInterface::ControlTransfer(
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
		*(this->handle),
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

BOOL WinUsbTowerInterface::Write(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthWritten) const
{
	BOOL success = WinUsb_WritePipe(
		*(this->handle),
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

BOOL WinUsbTowerInterface::Read(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthRead) const
{
	BOOL success = WinUsb_ReadPipe(
		*(this->handle),
		TOWER_READ_PIPE_ID,
		buffer,
		bufferLength,
		&lengthRead,
		NULL
	);

	PrintErrorIfAny("WinUSB read error");
#if DEBUG == 1
	if (!success)
		__debugbreak();
#endif

	return success;	
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
