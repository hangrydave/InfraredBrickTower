#pragma once

#include "HostTowerCommInterface.h"
#include <winusb.h>

class WinUsbTowerInterface : public HostTowerCommInterface
{
private:
	const WINUSB_INTERFACE_HANDLE* handle;

	VOID PrintErrorIfAny(const char* caller) const;
public:
	WinUsbTowerInterface(const WINUSB_INTERFACE_HANDLE* handle);

	BOOL ControlTransfer(
		BYTE request,
		WORD value,
		WORD index,
		USHORT bufferLength,
		BYTE* buffer,
		ULONG& lengthTransferred) const;

	BOOL Write(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten) const;

	BOOL Read(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead) const;
};

