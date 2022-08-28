#include "pch.h"

#include "TowerController.h"
#include "WinUsbTowerInterface.h"
#include "VLLCommands.h"
#include "LASM.h"

#include <assert.h>
#include <stdio.h>
#include <iostream>

BOOL StringsAreEqual(char* strOne, char* strTwo);
VOID MicroScoutCLI(Tower::TowerData* towerData);

VOID TestTower(Tower::TowerData* data);
VOID BeepRCX(Tower::TowerData* towerData);
VOID BeepMicroScout(Tower::TowerData* towerData);
VOID BeepRCXAndMicroScout(Tower::TowerData* towerData);

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

	Tower::TowerData* towerData = new Tower::TowerData(usbTowerInterface);

	BeepRCX(towerData);

	delete usbTowerInterface;
	delete towerData;

	system("pause");
	return 0;
}

VOID MicroScoutCLI(Tower::TowerData* data)
{
	enum MSCLIMode
	{
		DIRECT,
		PROGRAM
	};

	SetIndicatorLEDMode(Tower::TowerIndicatorLEDMode::HOST_SOFTWARE_CONTROLLED, data);
	SetMode(Tower::TowerMode::VLL, data);
	
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

VOID TestTower(Tower::TowerData* data)
{
	GetCopyright(data);
	wprintf(data->stringBuffer);

	GetCredits(data);
	wprintf(data->stringBuffer);
}

VOID BeepMicroScout(Tower::TowerData* towerData)
{
	Tower::SetMode(Tower::TowerMode::VLL, towerData);
	VLL_Beep1Immediate(towerData);
}

VOID BeepRCX(Tower::TowerData* towerData)
{
	SetMode(Tower::TowerMode::IR, towerData);

	ULONG lengthWritten;
	ULONG lengthRead = 0;
	UCHAR replyBuffer[10];
	ULONG replyLength = 10;

	BOOL writeSuccess;
	BOOL readSuccess;
	BOOL validateSuccess;

	LASM::CommandData command = LASM::Cmd_PBAliveOrNot();
	writeSuccess = Tower::WriteData(command.data.get(), command.dataLength, lengthWritten, towerData);
	readSuccess = Tower::ReadData(replyBuffer, replyLength, lengthRead, towerData);
	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
	assert(writeSuccess && readSuccess && validateSuccess);

	command = LASM::Cmd_StopAllTasks();
	writeSuccess = Tower::WriteData(command.data.get(), command.dataLength, lengthWritten, towerData);
	readSuccess = Tower::ReadData(replyBuffer, replyLength, lengthRead, towerData);
	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
	assert(writeSuccess && readSuccess && validateSuccess);

	command = LASM::Cmd_PlaySystemSound(LASM::BEEP);
	writeSuccess = Tower::WriteData(command.data.get(), command.dataLength, lengthWritten, towerData);
	readSuccess = Tower::ReadData(replyBuffer, replyLength, lengthRead, towerData);
	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
}

VOID BeepRCXAndMicroScout(Tower::TowerData* towerData)
{
	/* MicroScout */

	printf("Sending beep command to MicroScout...\n");
	BeepMicroScout(towerData);
	printf("Sent beep command to MicroScout!\n");

#if DRAMATIC_PAUSE == 1
	Sleep(1000);
	printf("\nPausing for dramatic effect...\n\n");
	Sleep(1500);
#endif

	/* RCX */
	printf("Sending beep command to RCX...\n");
	BeepRCX(towerData);
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
