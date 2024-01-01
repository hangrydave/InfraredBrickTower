#pragma once

#include "HostTowerCommInterface.h"
#include <Windows.h>

class WinUsbTowerInterface : public HostTowerCommInterface
{
private:

	// Look, I don't pretend to understand C++ very well...
	// I can't store a DEVICE_DATA variable in here for reasons I can't comprehend,
	// so in the meantime, I'll just store a copy of everything in there.
	BOOL                    handlesOpen;
	PVOID					winUsbHandle;
	HANDLE                  deviceHandle;
	TCHAR*                  devicePath;

#define MAX_TIMEOUT_COUNT 5
	UINT timeoutCounter;

	VOID PrintErrorIfAny(const char* caller) const;

public:
	WinUsbTowerInterface(
		BOOL handlesOpen,
		PVOID winUsbHandle,
		HANDLE deviceHandle,
		PTCHAR devicePath);

	~WinUsbTowerInterface();

	bool ControlTransfer(
		BYTE request,
		WORD value,
		WORD index,
		USHORT bufferLength,
		BYTE* buffer,
		ULONG& lengthTransferred) const;

	bool Write(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten) const;

	bool Read(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead);

	bool Flush() const;
};

BOOL OpenWinUsbTowerInterface(WinUsbTowerInterface*& towerInterface);
