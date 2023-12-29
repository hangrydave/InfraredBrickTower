#include "pch.h"
#include "WinUsbTowerInterface.h"
#include "TowerController.h"
#include "VLL.h"
#include "LASM.h"
#include "PBrick.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>

BOOL StringsAreEqual(char* strOne, char* strTwo);
VOID MicroScoutCLI(Tower::RequestData* towerData);

VOID TestTower(Tower::RequestData* data);
VOID DriveMotors(Tower::RequestData* towerData);
VOID BeepRCX(Tower::RequestData* towerData);
VOID BeepMicroScout(Tower::RequestData* towerData);
VOID BeepRCXAndMicroScout(Tower::RequestData* towerData);

int main()
{
	WinUsbTowerInterface* usbTowerInterface;
	BOOL gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
	if (!gotInterface)
	{
		printf("Error getting WinUSB interface!\n");
		system("pause");
		return 0;
	}

	Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	BYTE replyBuffer[64];
	ULONG lengthRead = 0;

	ULONG lengthWritten = 0;
	/*BOOL writeSuccess = Tower::WriteData(
		command.data,
		command.dataLength,
		lengthWritten,
		towerData);

	BOOL readSuccess = Tower::ReadData(
		replyBuffer,
		21,
		lengthRead,
		towerData);*/

	/*int rom_major = replyBuffer[7];
	int rom_minor = replyBuffer[10];

	int ram_major = replyBuffer[14];
	int ram_minor = replyBuffer[16] * 10 + replyBuffer[17];*/

	Tower::Flush(Tower::CommBuffer::ALL_BUFFERS, towerData);
	usbTowerInterface->Flush();

	//RCX::DownloadFirmware("C:\\Users\\david\\source\\repos\\LegoInfraredUSBTower\\ConsoleTester\\x64\\Debug\\firm0332.lgo", towerData);


	//assert(RCX::DownloadProgram("drive_until_button.rcx", 0, towerData));
	//assert(RCX::DownloadProgram("example_program.rcx", 1, towerData));
	//assert(RCX::DownloadProgram("beep.rcx", 2, towerData));
	//DriveMotors(towerData));
	BeepRCX(towerData);
	//MicroScoutCLI(towerData);

	while (true)
	{

		/*
		This was code testing the ability to read input from various lego IR remotes, and the results are promising!
		I was able to get input from both a Manas remote and (to my surprise) a Power Functions remote.
		*/
		//if (usbTowerInterface->Read(remoteReadBuffer, remoteBufferLen, remoteReadLength))
		//{
		//    if (remoteReadLength > 0)
		//    {
		//        printf("remote input, size %d: ", remoteReadLength);
		//        for (int i = 0; i < remoteReadLength; i++)
		//        {
		//            //printf("%#x", remoteReadBuffer[i]);
		//            std::cout << (int) (remoteReadBuffer[i]);
		//        }
		//        printf("\n");
		//    }
		//}
	}

	usbTowerInterface->Flush();

	delete usbTowerInterface;
	delete towerData;

	system("pause");
	return 0;
}

VOID DriveMotors(Tower::RequestData* towerData)
{
	LASM::CommandData command;
	LASM::Cmd_StopAllTasks(command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_PlayTone(255, 40, command);
	assert(LASM::SendCommand(&command, towerData));

	LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 6, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_SetFwdSetRwdRewDir(LASM::Motor::A | LASM::Motor::C, LASM::MotorDirection::FORWARDS, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::ON, command);
	assert(LASM::SendCommand(&command, towerData));

	Sleep(1000);

	LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 0, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::FLOAT, command);
	assert(LASM::SendCommand(&command, towerData));

	Sleep(1000);

	LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 6, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_SetFwdSetRwdRewDir(LASM::Motor::A | LASM::Motor::C, LASM::MotorDirection::REVERSE, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::ON, command);
	assert(LASM::SendCommand(&command, towerData));

	Sleep(1000);

	LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 0, command);
	assert(LASM::SendCommand(&command, towerData));
	LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::FLOAT, command);
	assert(LASM::SendCommand(&command, towerData));
}

VOID MicroScoutCLI(Tower::RequestData* towerData)
{
	enum MSCLIMode
	{
		DIRECT,
		PROGRAM
	};

	Tower::SetIndicatorLEDMode(Tower::IndicatorLEDMode::HOST_SOFTWARE_CONTROLLED, towerData);
	Tower::SetCommMode(Tower::CommMode::VLL, towerData);

	/*char* help = "Commands:\n\nquit, help\n\nbeep1, beep2, beep3, beep4, beep5\n\nfwd, bwd\n\nstop, run, delete\n\nwaitlight, seeklight, code, keepalive\n\nUse \"directmode\" for immediate control and \"programmode\" to program the MicroScout.\n";
	printf(help);

	BYTE commandBuffer[VLL_PACKET_LENGTH];
	char input[256];

	ULONG lengthWritten = 0;

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
				? VLL::Cmd_Beep1(commandBuffer)
				: VLL::Cmd_Beep1Immediate(commandBuffer);
		}
		else if (StringsAreEqual(input, "beep2"))
		{
			mode == PROGRAM
				? VLL::Cmd_Beep2(commandBuffer)
				: VLL::Cmd_Beep2Immediate(commandBuffer);
		}
		else if (StringsAreEqual(input, "beep3"))
		{
			mode == PROGRAM
				? VLL::Cmd_Beep3(commandBuffer)
				: VLL::Cmd_Beep3Immediate(commandBuffer);
		}
		else if (StringsAreEqual(input, "beep4"))
		{
			mode == PROGRAM
				? VLL::Cmd_Beep4(commandBuffer)
				: VLL::Cmd_Beep4Immediate(commandBuffer);
		}
		else if (StringsAreEqual(input, "beep5"))
		{
			mode == PROGRAM
				? VLL::Cmd_Beep5(commandBuffer)
				: VLL::Cmd_Beep5Immediate(commandBuffer);
		}
		else if (StringsAreEqual(input, "fwd"))
		{
			if (mode == DIRECT)
			{
				VLL::Cmd_ForwardImmediate(commandBuffer);
			}
			else if (mode == PROGRAM)
			{
				printf("enter half, one, two, or five:");
				scanf("%s", input);
				if (StringsAreEqual(input, "half"))
				{
					VLL::Cmd_ForwardHalf(commandBuffer);
				}
				else if (StringsAreEqual(input, "one"))
				{
					VLL::Cmd_ForwardOne(commandBuffer);
				}
				else if (StringsAreEqual(input, "two"))
				{
					VLL::Cmd_ForwardTwo(commandBuffer);
				}
				else if (StringsAreEqual(input, "five"))
				{
					VLL::Cmd_ForwardFive(commandBuffer);
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
				VLL::Cmd_BackwardImmediate(commandBuffer);
			}
			else if (mode == PROGRAM)
			{
				printf("enter half, one, two, or five:");
				scanf("%s", input);
				if (StringsAreEqual(input, "half"))
				{
					VLL::Cmd_BackwardHalf(commandBuffer);
				}
				else if (StringsAreEqual(input, "one"))
				{
					VLL::Cmd_BackwardOne(commandBuffer);
				}
				else if (StringsAreEqual(input, "two"))
				{
					VLL::Cmd_BackwardTwo(commandBuffer);
				}
				else if (StringsAreEqual(input, "five"))
				{
					VLL::Cmd_BackwardFive(commandBuffer);
				}
				else
				{
					printf("invalid; canceling bkwd command\n");
				}
			}
		}
		else if (StringsAreEqual(input, "stop"))
		{
			VLL::Cmd_Stop(commandBuffer);
		}
		else if (StringsAreEqual(input, "run"))
		{
			VLL::Cmd_Run(commandBuffer);
		}
		else if (StringsAreEqual(input, "delete"))
		{
			VLL::Cmd_Delete(commandBuffer);
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
			VLL::Cmd_WaitLight(commandBuffer);
		}
		else if (StringsAreEqual(input, "seeklight"))
		{
			VLL::Cmd_SeekLight(commandBuffer);
		}
		else if (StringsAreEqual(input, "code"))
		{
			VLL::Cmd_Code(commandBuffer);
		}
		else if (StringsAreEqual(input, "keepalive"))
		{
			VLL::Cmd_KeepAlive(commandBuffer);
		}
		else
		{
			printf("Unrecognized command, try again.\n");
		}
		assert(Tower::WriteData(commandBuffer, VLL_PACKET_LENGTH, lengthWritten, towerData));
	}*/

}

VOID TestTower(Tower::RequestData* data)
{
	GetCopyright(data);
	wprintf(data->stringBuffer);

	GetCredits(data);
	wprintf(data->stringBuffer);
}

VOID BeepMicroScout(Tower::RequestData* towerData)
{
	Tower::SetCommMode(Tower::CommMode::VLL, towerData);

	BYTE command[VLL_PACKET_LENGTH];
	VLL::Cmd_Beep1Immediate(command);

	ULONG lengthWritten = 0;
	assert(Tower::WriteData(command, VLL_PACKET_LENGTH, lengthWritten, towerData));
}

VOID BeepRCX(Tower::RequestData* towerData)
{
	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	LASM::CommandData command;
	LASM::Cmd_PBAliveOrNot(command);
	assert(LASM::SendCommand(&command, towerData));

	LASM::Cmd_StopAllTasks(command);
	assert(LASM::SendCommand(&command, towerData));

	LASM::Cmd_PlaySystemSound(LASM::SystemSound::BEEP, command);
	assert(LASM::SendCommand(&command, towerData));
}

VOID BeepRCXAndMicroScout(Tower::RequestData* towerData)
{
	/* MicroScout */

	printf("Sending beep command to MicroScout...\n");
	BeepMicroScout(towerData);
	printf("Sent beep command to MicroScout!\n");

	Sleep(1000);
	printf("\nPausing for dramatic effect...\n\n");
	Sleep(1500);

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
