#include "pch.h"
#include "PBrick.h"
#include "LASM.h"
#include <assert.h>

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

		// i need the length
		/*ULONG64 size = std::filesystem::file_size(filePath);

		CHAR* fileData = new CHAR[size];
		input.read(fileData, size);*/

#pragma pack(push, 1)
		struct FirmwareHeader
		{
			char filename[32];
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct FirmwareChunk
		{
			char header[6];
			char body[32];
			char footer[2];
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct FirmwareFooter
		{
			char footer[10];
		};
#pragma pack(pop)

#define CHUNK_COUNT 1600

#pragma pack(push, 1)
		struct Firmware
		{
			FirmwareHeader header;
			int chunkCount = 0;
			FirmwareChunk chunks[CHUNK_COUNT];
			FirmwareFooter footer;
		};
#pragma pack(pop)

		char file_data[MAX_FIRMWARE_LENGTH];
		Firmware firmware;

		char partData[42];
		char partIndex = 0;

		bool inHeader = true;
		bool inChunk = false;
		bool inFooter = false;
		bool done = false;

#define HEADER 0
#define CHUNK 1
#define FOOTER 2
		int currentPart = HEADER;

		int fileLength = 0;

		while (!done)
		{
			// these files have an even number of characters, so this should be safe
			char pair[2];
			input.get(pair[0]);
			input.get(pair[1]);

			fileLength += 2;

			bool atEndOfPart = (pair[0] == 0x0D && pair[1] == 0x0A) || input.eof();

			if (pair[0] == 'S' && pair[1] == '0')
			{
				currentPart = HEADER;
			}
			else if (pair[0] == 'S' && pair[1] == '1')
			{
				currentPart = CHUNK;
			}
			else if (pair[0] == 'S' && pair[1] == '9')
			{
				currentPart = FOOTER;
			}
			else if (!atEndOfPart)
			{
				partData[partIndex++] = pair[0];
				partData[partIndex++] = pair[1];
			}
			else if (atEndOfPart)
			{
				switch (currentPart)
				{
				case HEADER:
					firmware.header = *reinterpret_cast<FirmwareHeader*>(partData);
					break;
				case CHUNK:
				{
					firmware.chunks[firmware.chunkCount++] = *reinterpret_cast<FirmwareChunk*>(partData);
					break;
				}
				case FOOTER:
					firmware.footer = *reinterpret_cast<FirmwareFooter*>(partData);
					done = true;
					break;
				default:
					break;
				}
				
				partIndex = 0;
			}
		}

		input.close();

		int blockCount = 1; // starts at 1, goes up and then back to 0 for the footer
		int byteCount = 1600;

		int dataByteCount = firmware.chunkCount * 32;
		char* allDataBytes = new char[dataByteCount];
		for (int chunkIndex = 0; chunkIndex < firmware.chunkCount; chunkIndex++)
		{
			int startIndex = chunkIndex * 32;

			FirmwareChunk chunk = firmware.chunks[chunkIndex];
			for (int byteIndex = 0; byteIndex < 32; byteIndex++)
			{
				int actualIndex = startIndex + byteIndex;
				unsigned char byte = chunk.body[byteIndex];
				allDataBytes[actualIndex] = byte;
			}
		}

		// in the wireshark dumps there are 125 ContinueFirmwareDownload commands sent
		// there are also 400 data bytes in each one
		// where do these numbers come from?

		// TODO: find way to calculate this, or a justification for 400
		int dataByteCountPerCommand = 400;

		int commandCount = dataByteCount / dataByteCountPerCommand;
		int leftover = dataByteCount % dataByteCountPerCommand;
		if (leftover > 0)
		{
			commandCount++;
		}

		for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
		{
			int lengthForThisOne = dataByteCountPerCommand;
			if (commandIndex == commandCount - 1 && leftover > 0)
			{
				// we're at the end, and there are leftovers
				lengthForThisOne = leftover;
			}

			int dataByteStartIndex = commandIndex * lengthForThisOne;
			int dataByteSum = 0;

			std::string dataString = "";
			for (int byteIndex = 0; byteIndex < lengthForThisOne; byteIndex++)
			{
				int actualIndex = dataByteStartIndex + byteIndex;
				unsigned char byte = allDataBytes[actualIndex];

				dataString += byte;
				dataByteSum += byte;
			}

			/*
			
			TODO

			i need to get the hex values from the ascii that the hex translates to
			and work with that instead.

			*/
			int dataByteChecksum = dataByteSum % 256;
			printf("e");
		}

		printf("done");
	}
}