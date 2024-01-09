#include "pch.h"
#include "PBrick.h"
#include "LASM.h"
#include <assert.h>
#include <filesystem>
#include "Utilities.h"

namespace fs = std::filesystem;

namespace RCX
{
	/*

	Big questions!

	Is this type of file something that the NQC dev created?

	Is it something created by the official software?

	Not sure!

	*/

	bool ParseRCXFile(const char* filePath, RCXFile& file)
	{
		std::ifstream input(filePath, std::ios::binary);
		if (!input)
		{
			printf("RCX::ParseRCXFile: file not found");
			return false;
		}

		input.read(reinterpret_cast<char*>(&file), FILE_HEADER_LENGTH);

		file.chunks = new Chunk[file.chunkCount];
		for (int i = 0; i < file.chunkCount; i++)
		{
			input.read(reinterpret_cast<char*>(&file.chunks[i]), CHUNK_HEADER_LENGTH);
			Chunk* chunk = &file.chunks[i];

			chunk->data = new BYTE[chunk->length];
			input.read(reinterpret_cast<char*>(chunk->data), chunk->length);

			// from line 100 in rcxifile.h in the nqc project
			// also line 240 in RCX_Image.cpp
			int paddingBytes = (4 - chunk->length) & 3;
			input.seekg(paddingBytes, std::ios::cur);
		}

		file.symbols = new Symbol[file.symbolCount];
		for (int i = 0; i < file.symbolCount; i++)
		{
			input.read(reinterpret_cast<char*>(&file.symbols[i]), SYMBOL_HEADER_LENGTH);
			Symbol* symbol = &file.symbols[i];

			symbol->name = new char[symbol->length];
			input.read(symbol->name, symbol->length);

			// TODO: look at logic at line 261 onwards in RCX_Image.cpp
			// sometimes there may not be a symbol, so just do a seek instead
		}

		input.close();
		return true;
	}

	bool DownloadProgram(const char* filePath, BYTE programSlot, Tower::RequestData* towerData)
	{
#define _returnIfFalse(condition) \
if (!condition) \
{ \
	return false; \
}

#define CHUNK_DOWNLOAD_SIZE 20
		BYTE replyBuffer[CHUNK_DOWNLOAD_SIZE]{};
		unsigned long lengthRead = 0;

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

		for (int i = 0; i < rcxFile.chunkCount; i++)
		{
			RCX::Chunk* chunk = &rcxFile.chunks[i];

			LASM::Cmd_PBAliveOrNot(command);
			_returnIfFalse(LASM::SendCommand(&command, towerData));

			if (chunk->type == TASK_CHUNK_ID)
				LASM::Cmd_BeginOfTask(chunk->number, chunk->length, command);
			else if (chunk->type == SUB_CHUNK_ID)
				LASM::Cmd_BeginOfSub(chunk->number, chunk->length, command);

			_returnIfFalse(LASM::SendCommand(&command, towerData));

			int remainingDataSize = chunk->length;
			int sizeToSend = 0;
			int chunkSequenceNumber = 1;
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

		return true;
	}

	bool DownloadFirmware(const char* filePath, Tower::RequestData* towerData)
	{
		std::ifstream input(filePath, std::ios::binary);
		if (!input)
		{
			printf("RCX::DownloadFirmware: file not found");
			return false;
		}

		// line 112, nqc.cpp
#define MAX_FIRMWARE_LENGTH 65536
#define HEADER_BYTE_LENGTH 28
#define CHUNK_BYTE_LENGTH 22
#define FOOTER_BYTE_LENGTH 6

		fs::path path = filePath;
		int fileSize = fs::file_size(path);

		/*
		fileSize is in chars, and each char in the file is half a byte.
		So, divide that by 2, subtract the header and footer lengths, divide it by the chunk size, and add 1!
		We've got a chunk count. And we know that, after the header and before the footer, each chunk has 16 data bytes.
		Now we can get the total amount of data bytes in the file.
		*/
		int chunkCount = 1 + ((fileSize / 2) - HEADER_BYTE_LENGTH - FOOTER_BYTE_LENGTH) / CHUNK_BYTE_LENGTH;
		int dataByteCount = chunkCount * 16;

		// TODO: find way to calculate this, or a justification for 200
#define DATA_BYTE_COUNT_PER_COMMAND 200

		int commandCount = dataByteCount / DATA_BYTE_COUNT_PER_COMMAND;
		int leftover = dataByteCount % DATA_BYTE_COUNT_PER_COMMAND;
		if (leftover > 0)
		{
			commandCount++;
		}

		int sumForChecksum = 0;

		bool inChunk = false;

		/*
		"part" refers to a part in the firmware file.

		For example, a part opening with S0 indicates the header.
		S9 indicates the footer, and S1 indicates a chunk, which contains data bytes.
		
		So, partByteIndex is the index of which byte we are at within the part (starting after the opening 2 characters labeling the part type).
		*/
		int partByteIndex = 0;

		BYTE* cmdDataBytes = new BYTE[DATA_BYTE_COUNT_PER_COMMAND];
		LASM::CommandData* continueDownloadCommands = new LASM::CommandData[commandCount];

		int dataByteIndex = 0;
		for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
		{
			int dataByteCountForThisOne = DATA_BYTE_COUNT_PER_COMMAND;
			if (commandIndex == commandCount - 1 && leftover > 0)
			{
				// we're at the end, and there are leftovers
				dataByteCountForThisOne = leftover;
			}

			int dataBytesRetrieved = 0;
			while (dataBytesRetrieved < dataByteCountForThisOne)
			{
				char charPair[2];
				input.get(charPair[0]);
				input.get(charPair[1]);

				bool atEndOfPart = (charPair[0] == 0x0D && charPair[1] == 0x0A) || input.eof();

				if (charPair[0] == 'S' && charPair[1] == '1') // As mentioned earlier, S1 shows up in front of data chunk parts.
				{
					inChunk = true;
					partByteIndex = 0;
				}
				else if (atEndOfPart)
				{
					inChunk = false;
				}
				else if (inChunk)
				{
#define DATA_START 3
#define DATA_END 18
					/*
					The chunks look like this:
						- 3 header bytes
						- 16 data bytes
						- 1 footer byte
					We're interested in just the data bytes, so we know that:
						- the first data byte is the 4th byte in the chunk, so index 3
						- the last data byte is the 19th byte in the chunk, so index 18
					So we can just check that it's in the range, and bingo!
					*/

					bool byteIsADataByte = DATA_START <= partByteIndex && partByteIndex <= DATA_END;
					partByteIndex++;

					if (byteIsADataByte)
					{
						BYTE byte = Utilities::ReadByteFromTwoHexChars(charPair[0], charPair[1]);
						
#define CHECKSUM_BYTE_COUNT 19456
						if (dataByteIndex++ < CHECKSUM_BYTE_COUNT)
						{
							// This is to calculate the firmware checksum
							// (refer to page 92 of LASM doc)
							sumForChecksum += byte;
						}
						cmdDataBytes[dataBytesRetrieved++] = byte;
					}
				}
			}

			int blockCount = (commandIndex + 1) % commandCount;

			// Create the actual command to send later
			LASM::Cmd_Download(
				cmdDataBytes,
				blockCount,
				dataByteCountForThisOne,
				continueDownloadCommands[commandIndex]);
		}

		delete[] cmdDataBytes;
		input.close();


		/* Send firmware commands to RCX */
		LASM::CommandData command;
		LASM::Cmd_GoIntoBootMode(command);
		_returnIfFalse(LASM::SendCommand(
			&command,
			towerData,
			NULL,
			true));

		int firmwareChecksum = sumForChecksum % 65536;
		LASM::Cmd_BeginFirmwareDownload(firmwareChecksum, command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		for (int i = 0; i < commandCount; i++)
		{
			_returnIfFalse(LASM::SendCommand(&continueDownloadCommands[i], towerData));
		}
		delete[] continueDownloadCommands;

		LASM::Cmd_UnlockFirmware(command);
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		return true;
	}
}