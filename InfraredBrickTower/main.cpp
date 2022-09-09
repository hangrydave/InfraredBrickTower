#include "pch.h"

#include "WinUsbTowerInterface.h"
#include "TowerController.h"
#include "VLL.h"
#include "LASM.h"
#include "RCXFileParser.h"
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
VOID DownloadBeepDotRCX(Tower::RequestData* towerData);

BOOL SendData(LASM::CommandData* command, Tower::RequestData* towerData);

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

	Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

	DownloadBeepDotRCX(towerData);
	//DriveMotors(towerData);
	//BeepRCX(towerData);
	//MicroScoutCLI(towerData);
	
	delete usbTowerInterface;
	delete towerData;

	system("pause");
	return 0;
}

BOOL SendData(LASM::CommandData* command, Tower::RequestData* towerData)
{
	ULONG lengthRead = 0;
	BYTE replyBuffer[10];
	ULONG replyLength = 10;
	return Tower::SendData(command->data, command->dataLength, replyBuffer, replyLength, lengthRead, towerData);
}

BOOL AreArraysEqual(BYTE* a, BYTE* b, UINT lenA, UINT lenB)
{
	if (lenA != lenB) return FALSE;

	for (int i = 0; i < lenA; i++)
	{
		if (a[i] != b[i]) return FALSE;
	}

	return TRUE;
}

VOID DownloadBeepDotRCX(Tower::RequestData* towerData)
{
	Tower::SetCommMode(Tower::CommMode::IR, towerData);

	RCX::RCXFile* rcxFile = new RCX::RCXFile();
	RCX::ParseFile("beep.rcx", *rcxFile);

	LASM::CommandData command = LASM::Cmd_PBAliveOrNot();
	assert(SendData(&command, towerData));

	command = LASM::Cmd_StopAllTasks();
	assert(SendData(&command, towerData));

	command = LASM::Cmd_SelectProgram(0);
	assert(SendData(&command, towerData));

	command = LASM::Cmd_DeleteAllTasks();
	assert(SendData(&command, towerData));

	command = LASM::Cmd_DeleteAllSubs();
	assert(SendData(&command, towerData));

	/*int fileSize = 0;
	for (int i = 0; i < rcxFile->chunkCount; i++)
	{
		fileSize += rcxFile->chunks[i].length;
	}*/

#define CHUNK_DOWNLOAD_SIZE 20
	BYTE replyBuffer[CHUNK_DOWNLOAD_SIZE];
	ULONG lengthRead = 0;

	for (UINT i = 0; i < rcxFile->chunkCount; i++)
	{
		RCX::Chunk chunk = rcxFile->chunks[i];

		command = LASM::Cmd_PBAliveOrNot();
		assert(SendData(&command, towerData));

#define TASK_CHUNK 0
#define SUB_CHUNK 1
#define SOUND_CHUNK 2
#define ANIMATION_CHUNK 3

		BYTE operation = (BYTE)(chunk.type == TASK_CHUNK ? LASM::Command::BeginOfTask : LASM::Command::BeginOfSub);
		if (chunk.type == TASK_CHUNK)
			command = LASM::Cmd_BeginOfTask(chunk.number, chunk.length);
		else if (chunk.type == SUB_CHUNK)
			command = LASM::Cmd_BeginOfSub(chunk.number, chunk.length);
		SendData(&command, towerData); // might not have reply? idk

		UINT remainingDataSize = chunk.length;
		UINT sizeToSend = 0;
		UINT chunkSequenceNumber = 1;
		BYTE* chunkData = chunk.data;
		while (remainingDataSize > 0)
		{
			if (remainingDataSize <= CHUNK_DOWNLOAD_SIZE)
			{
				sizeToSend = remainingDataSize;
				chunkSequenceNumber = 0;
			}
			else
			{
				sizeToSend = CHUNK_DOWNLOAD_SIZE;
			}
			remainingDataSize -= sizeToSend;

			command = LASM::Cmd_Download(chunk.data, chunkSequenceNumber, sizeToSend);
			Tower::SendData(command.data, command.dataLength, replyBuffer, CHUNK_DOWNLOAD_SIZE, lengthRead, towerData);

			chunkData += sizeToSend;
		}
	}

	command = LASM::Cmd_PlaySystemSound(LASM::SystemSound::FAST_SWEEP_UP);
	assert(SendData(&command, towerData));
}

//VOID DriveMotors(Tower::RequestData* towerData)
//{
//	ULONG lengthRead = 0;
//	UCHAR replyBuffer[10];
//	ULONG replyLength = 10;
//
//	LASM::CommandData command = LASM::Cmd_StopAllTasks();
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_PlayTone(255, 40);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//
//	command = LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 6);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_SetFwdSetRwdRewDir(LASM::Motor::A | LASM::Motor::C, LASM::MotorDirection::FORWARDS);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::ON);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//
//	Sleep(1000);
//
//	command = LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 0);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::FLOAT);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//
//	Sleep(1000);
//
//	command = LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 6);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_SetFwdSetRwdRewDir(LASM::Motor::A | LASM::Motor::C, LASM::MotorDirection::REVERSE);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::ON);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//
//	Sleep(1000);
//
//	command = LASM::Cmd_SetPower(LASM::Motor::A | LASM::Motor::C, LASM::ParamSource::CONSTANT, 0);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//	command = LASM::Cmd_OnOffFloat(LASM::Motor::A | LASM::Motor::C, LASM::MotorAction::FLOAT);
//	Tower::SendData(command.data.get(), command.dataLength, replyBuffer, replyLength, lengthRead, towerData);
//}
//
//VOID MicroScoutCLI(Tower::RequestData* towerData)
//{
//	enum MSCLIMode
//	{
//		DIRECT,
//		PROGRAM
//	};
//
//	Tower::SetIndicatorLEDMode(Tower::IndicatorLEDMode::HOST_SOFTWARE_CONTROLLED, towerData);
//	Tower::SetCommMode(Tower::CommMode::VLL, towerData);
//	
//	char* help = "Commands:\n\nquit, help\n\nbeep1, beep2, beep3, beep4, beep5\n\nfwd, bwd\n\nstop, run, delete\n\nwaitlight, seeklight, code, keepalive\n\nUse \"directmode\" for immediate control and \"programmode\" to program the MicroScout.\n";
//	printf(help);
//
//	BYTE commandBuffer[VLL_PACKET_LENGTH];
//	char input[256];
//
//	MSCLIMode mode = DIRECT;
//	while (!StringsAreEqual(input, "quit"))
//	{
//		printf(">");
//		scanf("%s", input);
//
//		if (StringsAreEqual(input, "quit"))
//		{
//			return;
//		}
//		else if (StringsAreEqual(input, "help"))
//		{
//			printf(help);
//		}
//		else if (StringsAreEqual(input, "beep1"))
//		{
//			mode == PROGRAM
//				? VLL::Cmd_Beep1(commandBuffer)
//				: VLL::Cmd_Beep1Immediate(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "beep2"))
//		{
//			mode == PROGRAM
//				? VLL::Cmd_Beep2(commandBuffer)
//				: VLL::Cmd_Beep2Immediate(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "beep3"))
//		{
//			mode == PROGRAM
//				? VLL::Cmd_Beep3(commandBuffer)
//				: VLL::Cmd_Beep3Immediate(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "beep4"))
//		{
//			mode == PROGRAM
//				? VLL::Cmd_Beep4(commandBuffer)
//				: VLL::Cmd_Beep4Immediate(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "beep5"))
//		{
//			mode == PROGRAM
//				? VLL::Cmd_Beep5(commandBuffer)
//				: VLL::Cmd_Beep5Immediate(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "fwd"))
//		{
//			if (mode == DIRECT)
//			{
//				VLL::Cmd_ForwardImmediate(commandBuffer);
//			}
//			else if (mode == PROGRAM)
//			{
//				printf("enter half, one, two, or five:");
//				scanf("%s", input);
//				if (StringsAreEqual(input, "half"))
//				{
//					VLL::Cmd_ForwardHalf(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "one"))
//				{
//					VLL::Cmd_ForwardOne(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "two"))
//				{
//					VLL::Cmd_ForwardTwo(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "five"))
//				{
//					VLL::Cmd_ForwardFive(commandBuffer);
//				}
//				else
//				{
//					printf("invalid; canceling fwd command\n");
//				}
//			}
//		}
//		else if (StringsAreEqual(input, "bwd"))
//		{
//			if (mode == DIRECT)
//			{
//				VLL::Cmd_BackwardImmediate(commandBuffer);
//			}
//			else if (mode == PROGRAM)
//			{
//				printf("enter half, one, two, or five:");
//				scanf("%s", input);
//				if (StringsAreEqual(input, "half"))
//				{
//					VLL::Cmd_BackwardHalf(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "one"))
//				{
//					VLL::Cmd_BackwardOne(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "two"))
//				{
//					VLL::Cmd_BackwardTwo(commandBuffer);
//				}
//				else if (StringsAreEqual(input, "five"))
//				{
//					VLL::Cmd_BackwardFive(commandBuffer);
//				}
//				else
//				{
//					printf("invalid; canceling bkwd command\n");
//				}
//			}
//		}
//		else if (StringsAreEqual(input, "stop"))
//		{
//			VLL::Cmd_Stop(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "run"))
//		{
//			VLL::Cmd_Run(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "delete"))
//		{
//			VLL::Cmd_Delete(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "programmode"))
//		{
//			mode = PROGRAM;
//		}
//		else if (StringsAreEqual(input, "directmode"))
//		{
//			mode = DIRECT;
//		}
//		else if (StringsAreEqual(input, "waitlight"))
//		{
//			VLL::Cmd_WaitLight(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "seeklight"))
//		{
//			VLL::Cmd_SeekLight(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "code"))
//		{
//			VLL::Cmd_Code(commandBuffer);
//		}
//		else if (StringsAreEqual(input, "keepalive"))
//		{
//			VLL::Cmd_KeepAlive(commandBuffer);
//		}
//		else
//		{
//			printf("Unrecognized command, try again.\n");
//		}
//		Tower::WriteData(commandBuffer, VLL_PACKET_LENGTH, towerData);
//	}
//
//}
//
//VOID TestTower(Tower::RequestData* data)
//{
//	GetCopyright(data);
//	wprintf(data->stringBuffer);
//
//	GetCredits(data);
//	wprintf(data->stringBuffer);
//}
//
//VOID BeepMicroScout(Tower::RequestData* towerData)
//{
//	Tower::SetCommMode(Tower::CommMode::VLL, towerData);
//
//	BYTE command[VLL_PACKET_LENGTH];
//	VLL::Cmd_Beep1Immediate(command);
//	Tower::WriteData(command, VLL_PACKET_LENGTH, towerData);
//}
//
//VOID BeepRCX(Tower::RequestData* towerData)
//{
//	Tower::SetCommMode(Tower::CommMode::IR, towerData);
//
//	ULONG lengthWritten;
//	ULONG lengthRead = 0;
//	UCHAR replyBuffer[10];
//	ULONG replyLength = 10;
//
//	BOOL sendSuccess;
//	BOOL validateSuccess;
//
//	LASM::CommandData command = LASM::Cmd_PBAliveOrNot();
//	sendSuccess = Tower::SendData(
//		command.data.get(),
//		command.dataLength,
//		replyBuffer,
//		replyLength,
//		lengthRead,
//		towerData);
//	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
//	assert(sendSuccess && validateSuccess);
//
//	command = LASM::Cmd_StopAllTasks();
//	sendSuccess = Tower::SendData(
//		command.data.get(),
//		command.dataLength,
//		replyBuffer,
//		replyLength,
//		lengthRead,
//		towerData);
//	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
//	assert(sendSuccess && validateSuccess);
//
//	command = LASM::Cmd_PlaySystemSound(LASM::SystemSound::BEEP);
//	sendSuccess = Tower::SendData(
//		command.data.get(),
//		command.dataLength,
//		replyBuffer,
//		replyLength,
//		lengthRead,
//		towerData);
//	validateSuccess = LASM::ValidateReply(command.command, replyBuffer, replyLength);
//	assert(sendSuccess && validateSuccess);
//}
//
//VOID BeepRCXAndMicroScout(Tower::RequestData* towerData)
//{
//	/* MicroScout */
//
//	printf("Sending beep command to MicroScout...\n");
//	BeepMicroScout(towerData);
//	printf("Sent beep command to MicroScout!\n");
//
//#if DRAMATIC_PAUSE == 1
//	Sleep(1000);
//	printf("\nPausing for dramatic effect...\n\n");
//	Sleep(1500);
//#endif
//
//	/* RCX */
//	printf("Sending beep command to RCX...\n");
//	BeepRCX(towerData);
//	printf("Sent beep command to RCX!\n\n");
//}

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
