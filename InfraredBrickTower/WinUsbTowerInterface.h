#pragma once

//#include "HostTowerCommInterface.h"
//#include <winusb.h>
//
//class WinUsbTowerInterface : public HostTowerCommInterface
//{
//private:
//
//	// Look, I don't pretend to understand C++ very well...
//	// I can't store a DEVICE_DATA variable in here for reasons I can't comprehend,
//	// so in the meantime, I'll just store a copy of everything in there.
//	BOOL                    handlesOpen;
//	WINUSB_INTERFACE_HANDLE winUsbHandle;
//	HANDLE                  deviceHandle;
//	TCHAR*                  devicePath;
//
//	VOID PrintErrorIfAny(const char* caller) const;
//public:
//	WinUsbTowerInterface(
//		BOOL handlesOpen,
//		WINUSB_INTERFACE_HANDLE winUsbHandle,
//		HANDLE deviceHandle,
//		PTCHAR devicePath);
//
//	~WinUsbTowerInterface();
//
//	BOOL ControlTransfer(
//		BYTE request,
//		WORD value,
//		WORD index,
//		USHORT bufferLength,
//		BYTE* buffer,
//		ULONG& lengthTransferred) const;
//
//	BOOL Write(
//		PUCHAR buffer,
//		ULONG bufferLength,
//		ULONG& lengthWritten) const;
//
//	BOOL Read(
//		PUCHAR buffer,
//		ULONG bufferLength,
//		ULONG& lengthRead) const;
//};
//
//BOOL OpenWinUsbTowerInterface(WinUsbTowerInterface*& towerInterface);
