#include "pch.h"
#include "LASM.h"
#include "stdio.h"
#include "TowerController.h"

namespace LASM
{
	BOOL SendCommand(CommandData* command, Tower::RequestData* towerData)
	{
		ULONG lengthRead = 0;
		BYTE replyBuffer[COMMAND_REPLY_BUFFER_LENGTH];

		ULONG lengthWritten = 0;
		BOOL writeSuccess = Tower::WriteData(
			command->data,
			command->dataLength,
			lengthWritten,
			towerData);

		//printf("Length written: %d\n", lengthWritten);

		if (!writeSuccess)
			return FALSE;

		BOOL readSuccess = Tower::ReadData(
			replyBuffer,
			COMMAND_REPLY_BUFFER_LENGTH,
			lengthRead,
			towerData);

		//printf("Length read: %d\n", lengthRead);

		if (!readSuccess)
			return FALSE;

		return ValidateReply(command, replyBuffer, lengthRead);
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
		...			Parameters
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

		// now it's expected that there is a pattern like <complement> <command> <complement> <command>.
		// the presence of that will determine if the reply is good or not.
		// UPDATE: maybe better to check for just <complement> <command> instead
		return replyBuffer[complementIndex] == complement &&
			replyBuffer[complementIndex + 1] == commandByte;//&&
			//replyBuffer[complementIndex + 2] == complement &&
			//replyBuffer[complementIndex + 3] == commandByte;
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

	VOID Cmd_SetPower(BYTE motors, ParamSource powerSource, BYTE powerValue, CommandData& commandData)
	{
		BYTE params[3]{ motors, (BYTE)powerSource, powerValue};
		ComposeCommand(Command::SetPower, params, 3, commandData);
	}

	VOID Cmd_PlayTone(WORD frequency, BYTE duration, CommandData& commandData)
	{
		BYTE frequencyHi = (frequency & 0xff00) >> 8;
		BYTE frequencyLo = frequency & 0x00ff;
		BYTE params[3]{ frequencyLo, frequencyHi, duration };
		ComposeCommand(Command::PlayTone, params, 3, commandData);
	}

	VOID Cmd_BeginOfTask(BYTE taskNumber, BYTE taskSize, CommandData& commandData)
	{
		BYTE taskSizeHi = (taskSize & 0xff00) >> 8;
		BYTE taskSizeLo = taskSize & 0x00ff;
		BYTE params[5]{ 0, taskNumber, 0, taskSizeLo, taskSizeHi };
		ComposeCommand(Command::BeginOfTask, params, 5, commandData);
	}
	
	VOID Cmd_BeginOfSub(BYTE subNumber, BYTE subSize, CommandData& commandData)
	{
		BYTE subSizeHi = (subSize & 0xff00) >> 8;
		BYTE subSizeLo = subSize & 0x00ff;
		BYTE params[5]{ 0, subNumber, 0, subSizeLo, subSizeHi };
		ComposeCommand(Command::BeginOfSub, params, 5, commandData);
	}

	VOID Cmd_Download(BYTE* data, BYTE blockCount, BYTE byteCount, CommandData& commandData)
	{
		// look at RCX_Cmd::MakeDownload in the NQC code for reference

		BYTE blockCountHi = (blockCount & 0xff00) >> 8;
		BYTE blockCountLo = blockCount & 0x00ff;
		BYTE byteCountHi = (byteCount & 0xff00) >> 8;
		BYTE byteCountLo = byteCount & 0x00ff;
		
		BYTE paramCount = 5 + byteCount;
		BYTE* params = new BYTE[paramCount];
		BYTE* paramsPtr = params;
		*paramsPtr++ = blockCountLo;
		*paramsPtr++ = blockCountHi;
		*paramsPtr++ = byteCountLo;
		*paramsPtr++ = byteCountHi;

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