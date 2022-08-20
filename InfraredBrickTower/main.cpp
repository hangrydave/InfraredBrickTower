#include "pch.h"

#include "TowerController.h"
#include "WinUsbTowerInterface.h"
#include "VLLCommands.h"

#include <assert.h>
#include <stdio.h>
#include <iostream>

using namespace IBT;

BOOL StringsAreEqual(char* strOne, char* strTwo);
VOID MicroScoutCLI(ControllerData* controllerData);

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

	//TestTower(controllerData);

	/*TestTower(controllerData);
	BeepRCXAndMicroScout(controllerData);*/

	MicroScoutCLI(controllerData);

	delete usbTowerInterface;
	delete controllerData;

	system("pause");
	return 0;
}

VOID MicroScoutCLI(ControllerData* data)
{
	enum MSCLIMode
	{
		DIRECT,
		PROGRAM
	};

	SetIndicatorLEDMode(TowerIndicatorLEDMode::HOST_SOFTWARE_CONTROLLED, data);
	SetMode(TowerMode::VLL, data);
	
	char* help = "Commands:\n\nquit, help\n\nbeep1, beep2, beep3, beep4, beep5\n\nfwd, bwd\n\nstop, run, delete\n\nwaitlight, seeklight, code, keepalive\n\nUse \"directmode\" for immediate control and \"programmode\" to program the MicroScout.\n";
	printf(help);

	char input[256];

	MSCLIMode mode = DIRECT;
	while (!StringsAreEqual(input, "quit"))
	{
		printf(">");
		scanf("%s", input);

		if (StringsAreEqual(input, "quit"))
		{
			return;
		}
		else if (StringsAreEqual(input, "help"))
		{
			printf(help);
		}
		else if (StringsAreEqual(input, "beep1"))
		{
			mode == PROGRAM
				? VLL_Beep1(data)
				: VLL_Beep1Immediate(data);
		}
		else if (StringsAreEqual(input, "beep2"))
		{
			mode == PROGRAM
				? VLL_Beep2(data)
				: VLL_Beep2Immediate(data);
		}
		else if (StringsAreEqual(input, "beep3"))
		{
			mode == PROGRAM
				? VLL_Beep3(data)
				: VLL_Beep3Immediate(data);
		}
		else if (StringsAreEqual(input, "beep4"))
		{
			mode == PROGRAM
				? VLL_Beep4(data)
				: VLL_Beep4Immediate(data);
		}
		else if (StringsAreEqual(input, "beep5"))
		{
			mode == PROGRAM
				? VLL_Beep5(data)
				: VLL_Beep5Immediate(data);
		}
		else if (StringsAreEqual(input, "fwd"))
		{
			if (mode == DIRECT)
			{
				VLL_ForwardImmediate(data);
			}
			else if (mode == PROGRAM)
			{
				printf("enter half, one, two, or five:");
				scanf("%s", input);
				if (StringsAreEqual(input, "half"))
				{
					VLL_ForwardHalf(data);
				}
				else if (StringsAreEqual(input, "one"))
				{
					VLL_ForwardOne(data);
				}
				else if (StringsAreEqual(input, "two"))
				{
					VLL_ForwardTwo(data);
				}
				else if (StringsAreEqual(input, "five"))
				{
					VLL_ForwardFive(data);
				}
				else
				{
					printf("invalid; canceling fwd command\n");
				}
			}
		}
		else if (StringsAreEqual(input, "bwd"))
		{
			if (mode == DIRECT)
			{
				VLL_BackwardImmediate(data);
			}
			else if (mode == PROGRAM)
			{
				printf("enter half, one, two, or five:");
				scanf("%s", input);
				if (StringsAreEqual(input, "half"))
				{
					VLL_BackwardHalf(data);
				}
				else if (StringsAreEqual(input, "one"))
				{
					VLL_BackwardOne(data);
				}
				else if (StringsAreEqual(input, "two"))
				{
					VLL_BackwardTwo(data);
				}
				else if (StringsAreEqual(input, "five"))
				{
					VLL_BackwardFive(data);
				}
				else
				{
					printf("invalid; canceling bkwd command\n");
				}
			}
		}
		else if (StringsAreEqual(input, "stop"))
		{
			VLL_Stop(data);
		}
		else if (StringsAreEqual(input, "run"))
		{
			VLL_Run(data);
		}
		else if (StringsAreEqual(input, "delete"))
		{
			VLL_Delete(data);
		}
		else if (StringsAreEqual(input, "programmode"))
		{
			mode = PROGRAM;
		}
		else if (StringsAreEqual(input, "directmode"))
		{
			mode = DIRECT;
		}
		else if (StringsAreEqual(input, "waitlight"))
		{
			VLL_WaitLight(data);
		}
		else if (StringsAreEqual(input, "seeklight"))
		{
			VLL_SeekLight(data);
		}
		else if (StringsAreEqual(input, "code"))
		{
			VLL_Code(data);
		}
		else if (StringsAreEqual(input, "keepalive"))
		{
			VLL_KeepAlive(data);
		}
		else
		{
			printf("Unrecognized command, try again.\n");
		}
	}
}

VOID TestTower(ControllerData* data)
{
	GetCopyright(data);
	wprintf(data->stringBuffer);

	GetCredits(data);
	wprintf(data->stringBuffer);
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

BOOL StringsAreEqual(char* strOne, char* strTwo)
{
	INT strOneLen = strlen(strOne);
	INT strTwoLen = strlen(strTwo);

	INT len = strOneLen > strTwoLen ? strOneLen : strTwoLen;

	BOOL hasPassedWhitespaceOne = FALSE;
	BOOL hasPassedWhitespaceTwo = FALSE;

	INT strTwoIndex = 0;
	for (INT strOneIndex = 0; strOneIndex < len; strOneIndex++, strTwoIndex++)
	{
		char a = ' ';
		if (strOneIndex < strOneLen)
		{
			a = strOne[strOneIndex];
		}

		BOOL aIsWhitespace = a == ' ' || a == '\t';
		if (!aIsWhitespace)
		{
			hasPassedWhitespaceOne = TRUE;
		}

		char b = ' ';
		if (strTwoIndex < strTwoLen)
		{
			b = strTwo[strTwoIndex];
		}

		BOOL bIsWhitespace = b == ' ' || b == '\t';
		if (!bIsWhitespace)
		{
			hasPassedWhitespaceTwo = TRUE;
		}

		if (!hasPassedWhitespaceOne)
		{
			strTwoIndex--;
			continue;
		}

		if (!hasPassedWhitespaceTwo)
		{
			strOneIndex--;
			continue;
		}

		if (strOneIndex >= strOneLen && !bIsWhitespace)
		{
			return FALSE;
		}

		if (strTwoIndex >= strTwoLen && !aIsWhitespace)
		{
			return FALSE;
		}

		if (a != b && !aIsWhitespace && !bIsWhitespace)
		{
			return FALSE;
		}
	}

	return TRUE;
}
