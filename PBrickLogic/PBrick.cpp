#include "pch.h"
#include "PBrick.h"
#include "LASM.h"
#include <assert.h>

namespace RCX
{
	BOOL ParseFile(const CHAR* fileName, RCXFile& file)
	{
		std::ifstream input(fileName, std::ios::binary);
		if (!input)
		{
			printf("RCX::ParseFile: file not found");
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

	BOOL DownloadProgram(const CHAR* fileName, BYTE programSlot, Tower::RequestData* towerData)
	{
#define _returnIfFalse(condition) \
if (!condition) \
	return FALSE;

#define CHUNK_DOWNLOAD_SIZE 20
		BYTE replyBuffer[CHUNK_DOWNLOAD_SIZE];
		ULONG lengthRead = 0;

		RCX::RCXFile rcxFile;
		_returnIfFalse(RCX::ParseFile(fileName, rcxFile));

		LASM::CommandData command;
		LASM::Cmd_PBAliveOrNot(command);
		printf("Cmd_PBAliveOrNot\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_StopAllTasks(command);
		printf("Cmd_StopAllTasks\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_SelectProgram(programSlot, command);
		printf("Cmd_SelectProgram\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_DeleteAllTasks(command);
		printf("Cmd_DeleteAllTasks\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		LASM::Cmd_DeleteAllSubs(command);
		printf("Cmd_DeleteAllSubs\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		for (UINT i = 0; i < rcxFile.chunkCount; i++)
		{
			RCX::Chunk chunk = rcxFile.chunks[i];

			LASM::Cmd_PBAliveOrNot(command);
			printf("Cmd_PBAliveOrNot\n");
			_returnIfFalse(LASM::SendCommand(&command, towerData));

			if (chunk.type == TASK_CHUNK_ID)
				LASM::Cmd_BeginOfTask(chunk.number, chunk.length, command);
			else if (chunk.type == SUB_CHUNK_ID)
				LASM::Cmd_BeginOfSub(chunk.number, chunk.length, command);

			printf("Cmd_BeginOf\n");
			_returnIfFalse(LASM::SendCommand(&command, towerData));

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

				LASM::Cmd_Download(chunkData, chunkSequenceNumber++, sizeToSend, command);

				printf("Cmd_Download\n");
				_returnIfFalse(LASM::SendCommand(&command, towerData));

				chunkData += sizeToSend;
			}
		}

		LASM::Cmd_PlaySystemSound(LASM::SystemSound::FAST_SWEEP_UP, command);
		printf("Cmd_PlaySystemSound\n");
		_returnIfFalse(LASM::SendCommand(&command, towerData));

		return TRUE;
	}
}