#include "pch.h"
#include "PBrick.h"
#include "LASM.h"
#include <assert.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace RCX
{
	/*
	
	Big questions!

	Is this type of file something that the NQC dev created?

	Is it something created by the official software?

	Not sure!
	
	*/

	BOOL ParseRCXFile(const CHAR* filePath, RCXFile& file)
	{
		std::ifstream input(filePath, std::ios::binary);
		if (!input)
		{
			printf("RCX::ParseRCXFile: file not found");
			return FALSE;
		}

		input.read(reinterpret_cast<CHAR*>(&file), FILE_HEADER_LENGTH);

		file.chunks = new Chunk[file.chunkCount];
		for (UINT i = 0; i < file.chunkCount; i++)
		{
			input.read(reinterpret_cast<CHAR*>(&file.chunks[i]), CHUNK_HEADER_LENGTH);
			Chunk* chunk = &file.chunks[i];

			chunk->data = new BYTE[chunk->length];
			input.read(reinterpret_cast<CHAR*>(chunk->data), chunk->length);

			// from line 100 in rcxifile.h in the nqc project
			// also line 240 in RCX_Image.cpp
			UINT paddingBytes = (4 - chunk->length) & 3;
			input.seekg(paddingBytes, std::ios::cur);
		}

		file.symbols = new Symbol[file.symbolCount];
		for (UINT i = 0; i < file.symbolCount; i++)
		{
			input.read(reinterpret_cast<char*>(&file.symbols[i]), SYMBOL_HEADER_LENGTH);
			Symbol* symbol = &file.symbols[i];

			symbol->name = new CHAR[symbol->length];
			input.read(symbol->name, symbol->length);

			// TODO: look at logic at line 261 onwards in RCX_Image.cpp
			// sometimes there may not be a symbol, so just do a seek instead
		}

		input.close();
		return TRUE;
	}

	BOOL DownloadProgram(const CHAR* filePath, BYTE programSlot, Tower::RequestData* towerData)
	{
#define _returnIfFalse(condition) \
if (!condition) \
{ \
	return FALSE; \
}

#define CHUNK_DOWNLOAD_SIZE 20
		BYTE replyBuffer[CHUNK_DOWNLOAD_SIZE]{};
		ULONG lengthRead = 0;

		RCX::RCXFile rcxFile;
		_returnIfFalse(RCX::ParseRCXFile(filePath, rcxFile));

		LASM::CommandData command;
		LASM::Cmd_PBAliveOrNot(command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_SelectProgram(programSlot, command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_StopAllTasks(command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_DeleteAllTasks(command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_DeleteAllSubs(command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		for (UINT i = 0; i < rcxFile.chunkCount; i++)
		{
			RCX::Chunk* chunk = &rcxFile.chunks[i];

			LASM::Cmd_PBAliveOrNot(command);
			_returnIfFalse(LASM::SendCommand(&command, towerData));

			if (chunk->type == TASK_CHUNK_ID)
				LASM::Cmd_BeginOfTask(chunk->number, chunk->length, command);
			else if (chunk->type == SUB_CHUNK_ID)
				LASM::Cmd_BeginOfSub(chunk->number, chunk->length, command);

			_returnIfFalse(LASM::SendCommand(&command, towerData));

			UINT remainingDataSize = chunk->length;
			UINT sizeToSend = 0;
			UINT chunkSequenceNumber = 1;
			BYTE* chunkData = chunk->data;
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

				LASM::Cmd_Download(chunkData, chunkSequenceNumber++, sizeToSend, command);
				_returnIfFalse(LASM::SendCommand(&command, towerData));

				chunkData += sizeToSend;
			}
		}

		LASM::Cmd_PlaySystemSound(LASM::SystemSound::FAST_SWEEP_UP, command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		return TRUE;
	}

	BYTE GetValueFromPair(const BYTE a, const BYTE b)
	{
		BYTE result = 1;

		if (a >= '0' && a <= '9')
		{
			// 30 is the hex value of '1' on the ascii table
			result = (a - 48) * 16;
		}
		else if (a >= 'A' && a <= 'F')
		{
			// it's numeric
			result = 160 + ((a - 65) * 16);
		}

		if (b >= '0' && b <= '9')
		{
			// 30 is the hex value of '1' on the ascii table
			result += (b - 48);
		}
		else if (b >= 'A' && b <= 'F')
		{
			// it's numeric
			result += (b - 65) + 10;
		}

		return result;
	}

	BOOL DownloadFirmware(const CHAR* filePath, Tower::RequestData* towerData)
	{
		std::ifstream input(filePath, std::ios::binary);
		if (!input)
		{
			printf("RCX::DownloadFirmware: file not found");
			return FALSE;
		}

		// line 112, nqc.cpp
#define MAX_FIRMWARE_LENGTH 65536
#define HEADER_BYTE_COUNT 28
#define CHUNK_BYTE_COUNT 22
#define FOOTER_BYTE_COUNT 6

		fs::path path = filePath;
		int fileSize = fs::file_size(path);

		int chunkCount = 1 + ((fileSize / 2) - HEADER_BYTE_COUNT - FOOTER_BYTE_COUNT) / CHUNK_BYTE_COUNT;
		int dataByteCount = chunkCount * 16;

		// in the wireshark dumps there are 125 ContinueFirmwareDownload commands sent
		// there are also 200 data bytes in each one
		// where do these numbers come from?

		// TODO: find way to calculate this, or a justification for 200
		int dataByteCountPerCommand = 200;
		int commandCount = dataByteCount / dataByteCountPerCommand;
		int leftover = dataByteCount % dataByteCountPerCommand;
		if (leftover > 0)
		{
			commandCount++;
		}

		int startAddress = 0x8000; // Refer to page 92 of LASM doc
		int sumOfFirst19456Bytes = 0;
		int fileIndex = 0;

		BYTE* cmdDataBytes = new BYTE[dataByteCountPerCommand];
		char byteChars[2];

		LASM::CommandData* continueDownloadCommands = new LASM::CommandData[commandCount];

		for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
		{
			int lengthForThisOne = dataByteCountPerCommand;
			if (commandIndex == commandCount - 1 && leftover > 0)
			{
				// we're at the end, and there are leftovers
				lengthForThisOne = leftover;
			}

			int dataByteStartIndex = commandIndex * dataByteCountPerCommand;
			int cmdDataByteSum = 0;

			int cmdDataByteIndex = 0;

			bool done = false;
			bool inChunk = false;
			int partByteIndex = 0;
			while (!done)
			{
				input.get(byteChars[0]);
				input.get(byteChars[1]);
				BYTE byte = GetValueFromPair(byteChars[0], byteChars[1]);

				fileIndex += 2;

				switch (byteChars[0])
				{
				case 'S':
					switch (byteChars[1])
					{
					case '1':
						chunkCount++;
						inChunk = true;
						break;
					case '9':
						inChunk = false;
						break;
					default:
						inChunk = false;
						break;
					}
					break;
				case 0x0D:
					if (byteChars[1] == 0x0A)
					{
						partByteIndex = 0;
					}
					break;
				default:
					if (!inChunk)
					{
						break;
					}

#define DATA_START 3
#define DATA_END 18
					bool byteIsUseful = partByteIndex >= DATA_START && partByteIndex <= DATA_END;
					partByteIndex++;

					if (!byteIsUseful)
					{
						break;
					}

					cmdDataBytes[cmdDataByteIndex++] = byte;
					cmdDataByteSum += byte;

					if (cmdDataByteIndex == lengthForThisOne)
					{
						done = true;
					}

					if (dataByteStartIndex + cmdDataByteIndex <= 19455)
					{
						// This is to calculate the firmware checksum
						// (refer to page 92 of LASM doc)
						sumOfFirst19456Bytes += byte;
					}

					break;
				}
			}

			int blockCount = (commandIndex + 1) % commandCount;

			LASM::Cmd_Download(
				cmdDataBytes,
				blockCount,
				lengthForThisOne,
				continueDownloadCommands[commandIndex]);

			for (int i = 0; i < continueDownloadCommands[commandIndex].dataLength; i++)
			{
				char c = continueDownloadCommands[commandIndex].data[i];
				printf("%02hhx", c);
			}
			printf("\n");
		}

		delete[] byteChars;
		delete[] cmdDataBytes;

		input.close();


		ULONG lengthRead = 0;
		BYTE* replyBuffer = new BYTE[COMMAND_REPLY_BUFFER_LENGTH];

		// TODO: clean up calls to SendCommand

		LASM::CommandData command;
		LASM::Cmd_GoIntoBootMode(command);
		_returnIfFalse(LASM::SendCommand(
			&command,
			towerData,
			replyBuffer,
			COMMAND_REPLY_BUFFER_LENGTH,
			true));

		int firmwareChecksum = sumOfFirst19456Bytes % 65536;
		LASM::Cmd_BeginFirmwareDownload(firmwareChecksum, command);
		_returnIfFalse(LASM::SendCommand(
			&command,
			towerData,
			replyBuffer,
			COMMAND_REPLY_BUFFER_LENGTH,
			true));

		ULONG lengthWritten = 0;
		for (int i = 0; i < commandCount; i++)
		{
			_returnIfFalse(LASM::SendCommand(
				&continueDownloadCommands[i],
				towerData, 
				replyBuffer,
				10, 
				true));
		}

		LASM::Cmd_UnlockFirmware(command);
		_returnIfFalse(LASM::SendCommand(
			&command,
			towerData,
			replyBuffer,
			COMMAND_REPLY_BUFFER_LENGTH,
			true));

		delete[] continueDownloadCommands;
		delete[] replyBuffer;

		return true;
 	}
}