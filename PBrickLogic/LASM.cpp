#include "pch.h"
#include "LASM.h"
#include "stdio.h"
#include "TowerController.h"

namespace LASM
{
#define LO_BYTE(b) b & 0x00ff
#define HI_BYTE(b) (b & 0xff00) >> 8

	BOOL SendCommand(CommandData* command, Tower::RequestData* towerData, ULONG expectedReplyLength)
	{
		ULONG lengthWritten = 0;
		BOOL writeSuccess = Tower::WriteData(
			command->data,
			command->dataLength,
			lengthWritten,
			towerData);

		if (!writeSuccess)
			return FALSE;

		if (expectedReplyLength > 0)
		{
			ULONG lengthRead = 0;
			BYTE replyBuffer[COMMAND_REPLY_BUFFER_LENGTH];
			BOOL readSuccess = Tower::ReadData(
				replyBuffer,
				COMMAND_REPLY_BUFFER_LENGTH,
				lengthRead,
				towerData);

			if (!readSuccess)
				return FALSE;

			return ValidateReply(command, replyBuffer, lengthRead);
		}

		return TRUE;
	}

	BOOL ValidateReply(CommandData* command, BYTE* replyBuffer, UINT replyLength)
	{
		/*
		
		NOTES TO SELF


		The format of these commands is

		0			0x55  \
		1			0xff   | Preamble
		2			0x00  /
		3			Command
		4			Complement of command
		...			Parameters (each byte followed by its complement)
		n - 2		Checksum
		n - 1		Complement of checksum

		Every byte after the preamble is followed immediately by its complement.

		*/

		BYTE commandByte = command->commandByte;
		BYTE complement = ~commandByte & 0xff;

		// first off, it can't be guaranteed that the typical preamble of 0x55 0xFF 0x00 will be there;
		// occasionally, just a part of it will be there.
		// so, i'll search through the buffer for the complement.
		UINT complementIndex = -1;
		for (UINT i = 0; i < replyLength; i++)
		{
			BYTE b = replyBuffer[i];
			if (b == complement)
			{
				complementIndex = i;
				break;
			}
		}

		if (complementIndex == -1)
		{
			return FALSE;
		}

		// now it's expected that there is a pattern like <complement> <command>.
		// the presence of that will determine if the reply is good or not
		return replyBuffer[complementIndex] == complement &&
			replyBuffer[complementIndex + 1] == commandByte;
	}

	VOID GetCommandFromCode(const char* code, BYTE* params, ULONG paramCount, CommandData* command)
	{
		// TODO: this is bad
		BYTE paramA = params[0];
		BYTE paramB = params[1];
		BYTE paramC = params[2];
		BYTE paramD = params[3];
		BYTE paramE = params[4];

		if (strcmp(code, "ping") == 0)
		{
			Cmd_PBAliveOrNot(*command);
		}
		else if (strcmp(code, "memmap") == 0)
		{
			Cmd_MemMap(*command);
		}
		else if (strcmp(code, "pollb") == 0)
		{
			Cmd_PBBattery(*command);
		}
		else if (strcmp(code, "delt") == 0)
		{
			Cmd_DeleteAllTasks(*command);
		}
		else if (strcmp(code, "stop") == 0)
		{
			Cmd_StopAllTasks(*command);
		}
		else if (strcmp(code, "offp") == 0)
		{
			Cmd_PBTurnOff(*command);
		}
		else if (strcmp(code, "dels") == 0)
		{
			Cmd_DeleteAllSubs(*command);
		}
		else if (strcmp(code, "playz") == 0)
		{
			Cmd_ClearSound(*command);
		}
		else if (strcmp(code, "msgz") == 0)
		{
			Cmd_ClearPBMessage(*command);
		}
		else if (strcmp(code, "monax") == 0)
		{
			Cmd_ExitAccessControl(*command);
		}
		else if (strcmp(code, "monex") == 0)
		{
			Cmd_ExitEventCheck(*command);
		}
		else if (strcmp(code, "mute") == 0)
		{
			Cmd_MuteSound(*command);
		}
		else if (strcmp(code, "speak") == 0)
		{
			Cmd_UnmuteSound(*command);
		}
		else if (strcmp(code, "dele") == 0)
		{
			Cmd_ClearAllEvents(*command);
		}
		else if (strcmp(code, "rets") == 0)
		{
			Cmd_EndOfSub(*command);
		}
		else if (strcmp(code, "out") == 0)
		{
			Cmd_OnOffFloat(paramA, (MotorAction)paramB, *command);
		}
		else if (strcmp(code, "txs") == 0)
		{
			//Cmd_PbTXPower((IRTransmissionRange)paramA, *command);
		}
		else if (strcmp(code, "plays") == 0)
		{
			Cmd_PlaySystemSound((SystemSound)paramA, *command);
		}
		else if (strcmp(code, "delt") == 0)
		{
			//Cmd_DeleteTask(paramA, *command);
		}
		else if (strcmp(code, "start") == 0)
		{
			//Cmd_StartTask(paramA, *command);
		}
		else if (strcmp(code, "stop") == 0)
		{
			//Cmd_StopTask(paramA, *command);
		}
		else if (strcmp(code, "prgm") == 0)
		{
			Cmd_SelectProgram(paramA, *command);
		}
		else if (strcmp(code, "tmrz") == 0)
		{
			//Cmd_ClearTimer(paramA, *command);
		}
		else if (strcmp(code, "tout") == 0)
		{
			//Cmd_PBPowerDownTime(paramA, *command);
		}
		else if (strcmp(code, "dels") == 0)
		{
			//Cmd_DeleteSub(paramA, *command);
		}
		else if (strcmp(code, "senz") == 0)
		{
			//Cmd_ClearSensorValue(paramA, *command);
		}
		else if (strcmp(code, "dir") == 0)
		{
			Cmd_SetFwdSetRwdRewDir(paramA, (MotorDirection)paramB, *command);
		}
		else if (strcmp(code, "calls") == 0)
		{
			//Cmd_GoSub(paramA, *command);
		}
		else if (strcmp(code, "jmp") == 0)
		{
			//Cmd_SJump(paramA, (JumpDirection)paramB, *command);
		}
		else if (strcmp(code, "loopc") == 0)
		{
			//Cmd_SCheckLoopCounter(paramA, *command);
		}
		else if (strcmp(code, "gout") == 0)
		{
			//Cmd_ConnectDisconnect(paramA, (MotorAction)paramB, *command);
		}
		else if (strcmp(code, "gdir") == 0)
		{
			//Cmd_SetNormSetInvAltDir()
		}
		else if (strcmp(code, "cnti") == 0)
		{
			//Cmd_IncCounter(paramA, *command);
		}
		else if (strcmp(code, "cntd") == 0)
		{
			//Cmd_DecCounter(paramA, *command);
		}
		else if (strcmp(code, "cntz") == 0)
		{
			//Cmd_ClearCounter(paramA, *command);
		}
		else if (strcmp(code, "setp") == 0)
		{
			//Cmd_SetPriority(paramA, *command);
		}
		else if (strcmp(code, "msgs") == 0)
		{
			//Cmd_InternMessage(paramA, *command);
		}
		else if (strcmp(code, "playv") == 0)
		{
			//Cmd_PlayToneVar(paramA, paramB, *command);
		}
		else if (strcmp(code, "poll") == 0)
		{
			//Cmd_Poll(paramA, paramB, *command);
		}
		else if (strcmp(code, "setw") == 0)
		{
			//Cmd_SetWatch(paramA, paramB, *command);
		}
		else if (strcmp(code, "sent") == 0)
		{
			//Cmd_SetSensorType(paramA, (SensorType)paramB, *command);
		}
		else if (strcmp(code, "senm") == 0)
		{
			//Cmd_SetSensorMode(paramA, paramB, (SensorMode)paramC, *command);
		}
		else if (strcmp(code, "logz") == 0)
		{

		}
		else if (strcmp(code, "log") == 0)
		{

		}
		else if (strcmp(code, "jmpl") == 0)
		{

		}
		else if (strcmp(code, "loops") == 0)
		{

		}
		else if (strcmp(code, "loopcl") == 0)
		{

		}
		else if (strcmp(code, "msg") == 0)
		{

		}
		else if (strcmp(code, "uart") == 0)
		{

		}
		else if (strcmp(code, "remote") == 0)
		{

		}
		else if (strcmp(code, "decvjn") == 0)
		{

		}
		else if (strcmp(code, "event") == 0)
		{

		}
		else if (strcmp(code, "pwr") == 0)
		{
			Cmd_SetPower(paramA, (ParamSource)paramB, paramC, *command);
		}
		else if (strcmp(code, "playt") == 0)
		{
			Cmd_PlayTone(*reinterpret_cast<WORD*>(params), paramC, *command);
		}
		else if (strcmp(code, "view") == 0)
		{

		}
		else if (strcmp(code, "wait") == 0)
		{

		}
		else if (strcmp(code, "pollm") == 0)
		{

		}
		else if (strcmp(code, "monal") == 0)
		{

		}
		else if (strcmp(code, "sete") == 0)
		{

		}
		else if (strcmp(code, "gpwr") == 0)
		{

		}
		else if (strcmp(code, "decvjnl") == 0)
		{

		}
		else if (strcmp(code, "cale") == 0)
		{

		}
		else if (strcmp(code, "setv") == 0)
		{

		}
		else if (strcmp(code, "sumv") == 0)
		{

		}
		else if (strcmp(code, "subv") == 0)
		{

		}
		else if (strcmp(code, "divv") == 0)
		{

		}
		else if (strcmp(code, "mulv") == 0)
		{

		}
		else if (strcmp(code, "sgnv") == 0)
		{

		}
		else if (strcmp(code, "absv") == 0)
		{

		}
		else if (strcmp(code, "andv") == 0)
		{

		}
		else if (strcmp(code, "orv") == 0)
		{

		}
		else if (strcmp(code, "polld") == 0)
		{

		}
		else if (strcmp(code, "mone") == 0)
		{

		}
		else if (strcmp(code, "set") == 0)
		{

		}
		else if (strcmp(code, "pollp") == 0)
		{

		}
		else if (strcmp(code, ";") == 0)
		{

		}
		else if (strcmp(code, ";") == 0)
		{

		}
		else if (strcmp(code, ";") == 0)
		{

		}
		else if (strcmp(code, "reset") == 0)
		{

		}
		else if (strcmp(code, ";") == 0)
		{

		}
		else if (strcmp(code, "chk") == 0)
		{

		}
		else if (strcmp(code, "chkl") == 0)
		{

		}
		else if (strcmp(code, "boot") == 0)
		{

		}
		else if (strcmp(code, "monel") == 0)
		{

		}
		else if (strcmp(code, "disp") == 0)
		{

		}
	}

	VOID Cmd_OnOffFloat(BYTE motors, MotorAction action, CommandData& commandData)
	{
		BYTE actionBits = (BYTE)action << 6;
		BYTE params[1]{ actionBits | motors };
		ComposeCommand(Command::OnOffFloat, params, 1, commandData);
	}

	VOID Cmd_PlaySystemSound(SystemSound sound, CommandData& commandData)
	{
		BYTE params[1]{ (BYTE)sound };
		ComposeCommand(Command::PlaySystemSound, params, 1, commandData);
	}

	VOID Cmd_SelectProgram(BYTE program, CommandData& commandData)
	{
		BYTE params[1]{ (BYTE)program };
		ComposeCommand(Command::SelectProgram, params, 1, commandData);
	}

	VOID Cmd_RemoteCommand(WORD request, CommandData& commandData)
	{
		BYTE params[2]{ LO_BYTE((WORD)request), HI_BYTE((WORD)request) };
		ComposeCommand(Command::RemoteCommand, params, 2, commandData);
	}

	VOID Cmd_SetPower(BYTE motors, ParamSource powerSource, BYTE powerValue, CommandData& commandData)
	{
		BYTE params[3]{ motors, (BYTE)powerSource, powerValue};
		ComposeCommand(Command::SetPower, params, 3, commandData);
	}

	VOID Cmd_PlayTone(WORD frequency, BYTE duration, CommandData& commandData)
	{
		BYTE params[3]{ LO_BYTE(frequency), HI_BYTE(frequency), duration };
		ComposeCommand(Command::PlayTone, params, 3, commandData);
	}

	VOID Cmd_BeginOfTask(BYTE taskNumber, BYTE taskSize, CommandData& commandData)
	{
		BYTE params[5]{ 0, taskNumber, 0, LO_BYTE(taskSize), HI_BYTE(taskSize)};
		ComposeCommand(Command::BeginOfTask, params, 5, commandData);
	}
	
	VOID Cmd_BeginOfSub(BYTE subNumber, BYTE subSize, CommandData& commandData)
	{
		BYTE params[5]{ 0, subNumber, 0, LO_BYTE(subSize), HI_BYTE(subSize)};
		ComposeCommand(Command::BeginOfSub, params, 5, commandData);
	}

	VOID Cmd_Download(BYTE* data, BYTE blockCount, BYTE byteCount, CommandData& commandData)
	{
		// look at RCX_Cmd::MakeDownload in the NQC code for reference
		
		BYTE paramCount = 5 + byteCount;
		BYTE* params = new BYTE[paramCount];
		BYTE* paramsPtr = params;
		*paramsPtr++ = LO_BYTE(blockCount);
		*paramsPtr++ = HI_BYTE(blockCount);
		*paramsPtr++ = LO_BYTE(byteCount);
		*paramsPtr++ = HI_BYTE(byteCount);

		BYTE* dataPtr = data;
		BYTE blockChecksum = 0;
		BYTE bytesLeft = byteCount;
		while (bytesLeft > 0)
		{
			BYTE b = *dataPtr++;
			blockChecksum += b;
			*paramsPtr++ = b;
			bytesLeft--;
		}

		*paramsPtr = blockChecksum;
		ComposeCommand(Command::Download, params, paramCount, commandData);
	}

	VOID Cmd_SetFwdSetRwdRewDir(BYTE motors, MotorDirection direction, CommandData& commandData)
	{
		BYTE directionBits = (BYTE)direction << 6;
		BYTE params[1]{ directionBits | motors };
		ComposeCommand(Command::SetFwdSetRwdRewDir, params, 1, commandData);
	}

	VOID ComposeCommand(Command lasmCommand, BYTE* params, UINT paramsLength, CommandData& commandData)
	{
		UINT index = 0;

		// preamble
		commandData.data[index++] = 0x55;
		commandData.data[index++] = 0xFF;
		commandData.data[index++] = 0x00;

		UINT dataSum = 0;

		// command, reply, and repeat both
		commandData.previousCommandByte = commandData.commandByte;
		commandData.commandByte = (BYTE)lasmCommand;
		
		if (commandData.previousCommandByte == commandData.commandByte)
			commandData.commandByte ^= 8; // refer to line 252 in RCX_PipeTransport.cpp from NQC

		commandData.data[index++] = commandData.commandByte;
		commandData.data[index++] = ~commandData.commandByte;

		dataSum += commandData.commandByte;

		for (UINT i = 0; i < paramsLength; i++)
		{
			BYTE paramByte = params[i];
			commandData.data[index++] = paramByte;
			commandData.data[index++] = ~paramByte;

			dataSum += paramByte;
		}

		// checksum for the RCX is just the data sum, so...
		commandData.data[index++] = dataSum;
		commandData.data[index++] = ~dataSum;

		commandData.dataLength = index;
	}
}