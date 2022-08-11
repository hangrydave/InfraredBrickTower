#include "WinUsbTowerInterface.h"
#include <stdio.h>

WinUsbTowerInterface::WinUsbTowerInterface(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;

	ULONG timeoutBuffer[1] = { 1000 };
	BOOL policySetResult = WinUsb_SetPipePolicy(
		*handle,
		TOWER_WRITE_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutBuffer
	);
	if (!policySetResult)
	{
		printf("Failed to set timeout for pipe 2 (write)");
	}

	policySetResult = WinUsb_SetPipePolicy(
		*handle,
		TOWER_READ_PIPE_ID,
		PIPE_TRANSFER_TIMEOUT,
		8,
		timeoutBuffer
	);
	if (!policySetResult)
	{
		printf("Failed to set timeout for pipe 129 (read)");
	}
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

	PrintErrorIfAny("WinUsbTowerInterface::ControlInterface");

	return success;
}

VOID WinUsbTowerInterface::PrintErrorIfAny(const char* caller) const
{
	DWORD error = GetLastError();
	switch (error)
	{
	case ERROR_INVALID_HANDLE:
		printf("\n%s: invalid WinUsb handle provided", caller);
		break;
	case ERROR_IO_PENDING:
		printf("\n%s: overlapped I\\O operation in progress, operation executing in background", caller);
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
		printf("\n%s: not enough memory", caller);
		break;
	case ERROR_SEM_TIMEOUT:
		printf("\n%s: timeout", caller);
		break;
	default:
		break;
	}
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

	PrintErrorIfAny("WinUsbTowerInterface::Write");

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

	PrintErrorIfAny("WinUsbTowerInterface::Read");

	return success;	
}
