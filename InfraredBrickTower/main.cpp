#include "pch.h"

#include "TowerController.h"
#include "WinUsbTowerInterface.h"
#include "VLLCommands.h"

#include <assert.h>
#include <stdio.h>
#include <iostream>

using namespace IBT;

VOID TestTower(ControllerData* data);

VOID BeepRCXAndMicroScout(ControllerData* controllerData);

LONG __cdecl _tmain(LONG Argc, LPTSTR* Argv)
{
	UNREFERENCED_PARAMETER(Argc);
	UNREFERENCED_PARAMETER(Argv);

	WinUsbTowerInterface* usbTowerInterface;
	BOOL gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
	if (!gotInterface)
	{
		printf("Error getting WinUSB interface!\n");
		system("pause");
		return 0;
	}

	ControllerData* controllerData = new ControllerData(usbTowerInterface);

	TestTower(controllerData);
	BeepRCXAndMicroScout(controllerData);

	delete usbTowerInterface;
	delete controllerData;

	system("pause");
	return 0;
}

VOID TestTower(ControllerData* data)
{
	SetIndicatorLEDMode(TowerIndicatorLEDMode::HOST_SOFTWARE_CONTROLLED, data);
	SetLEDColor(TowerLED::VLL, TowerLEDColor::DEFAULT, data);
	SetLEDColor(TowerLED::VLL, TowerLEDColor::OFF, data);

	INT len;
	CHAR* buffer = 0;

	GetCopyright(buffer, len, data);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}

	GetCredits(buffer, len, data);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buffer[i]);
	}
}

VOID BeepRCXAndMicroScout(ControllerData* controllerData)
{
	/* MicroScout */

	printf("Sending beep command to MicroScout...\n");
	IBT::SetMode(TowerMode::VLL, controllerData);
	VLL_Beep1Immediate(controllerData);
	printf("Sent beep command to MicroScout!\n");

#if DRAMATIC_PAUSE == 1
	Sleep(1000);
	printf("\nPausing for dramatic effect...\n\n");
	Sleep(1500);
#endif

	/* RCX */
	printf("Sending beep command to RCX...\n");
	SetMode(TowerMode::IR, controllerData);

	UCHAR replyBuffer[10];
	ULONG replyLen = 10;
	BYTE replyByte;

	ULONG bytesWritten = 0;
	ULONG bytesRead = 0;

	UCHAR ping[] = { 0x55, 0xff, 0x00, 0x10, 0xef, 0x10, 0xef };
	ULONG pingLen = 7;
	WriteData(ping, pingLen, bytesWritten, controllerData);
	ReadData(replyBuffer, replyLen, bytesRead, controllerData);
	replyByte = *(replyBuffer + 3) & 0xf7;

#if DEBUG == 1
	if (replyByte != 0xE7)
		__debugbreak();
#endif

	UCHAR stop[] = { 0x55, 0xff, 0x00, 0x50, 0xaf, 0x50, 0xaf };
	ULONG stopLen = 7;
	WriteData(stop, stopLen, bytesWritten, controllerData);
	ReadData(replyBuffer, replyLen, bytesRead, controllerData);
	replyByte = *(replyBuffer + 3) & 0xf7;

#if DEBUG == 1
	if (replyByte != 0xA7)
		__debugbreak();
#endif

	UCHAR beep[] = { 0x55, 0xff, 0x00, 0x51, 0xae, 0x05, 0xfa, 0x56, 0xa9 };
	ULONG beepLen = 9;
	// expects 0xA6
	WriteData(beep, beepLen, bytesWritten, controllerData);
	ReadData(replyBuffer, replyLen, bytesRead, controllerData);
	replyByte = *(replyBuffer + 3) & 0xf7;

#if DEBUG == 1
	if (replyByte != 0xA6)
		__debugbreak();
#endif

	printf("Sent beep command to RCX!\n\n");
}
