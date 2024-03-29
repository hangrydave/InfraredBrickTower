#pragma once

#include <fstream>

#if defined(WIN32)

#include <stdio.h>

#elif defined(__linux)

#include <cstdio>

#endif

#include "TowerController.h"

#define BYTE unsigned char
#define WORD unsigned short

/*

A .rcx file looks something like this:

>>> header <<<
file signature
rcx firmware version?
chunk count
symbol count
target type (type of pbrick i think)
reserved byte

>>> chunks (as many as specified above) <<<
>>> chunk definition <<<
type
number		// TODO: find more definition on these
length
data (size is length)

>>> symbols (as many as specified in header) <<<
>>> symbol definition <<<
type
index
length
reserved
name (length is specified by the length field)

*/

namespace RCX
{
#define TASK_CHUNK_ID 0
#define SUB_CHUNK_ID 1
#define SOUND_CHUNK_ID 2
#define ANIMATION_CHUNK_ID 3

#pragma pack(push, 1)
	struct Chunk
	{
		BYTE type;
		BYTE number;
		WORD length;

		BYTE* data;

		~Chunk()
		{
			delete[] data;
		}
	};
#pragma pack(pop)
#define CHUNK_HEADER_LENGTH 4

#pragma pack(push, 1)
	struct Symbol
	{
		BYTE type;
		BYTE index;
		BYTE length;
		BYTE reserved;

		char* name;

		~Symbol()
		{
			delete[] name;
		}
	};
#pragma pack(pop)
#define SYMBOL_HEADER_LENGTH 4

#pragma pack(push, 1)
	struct RCXFile
	{
		unsigned int signature;
		unsigned short version;
		unsigned short chunkCount;
		unsigned short symbolCount;
		BYTE targetType;
		BYTE reserved;

		Chunk* chunks;
		Symbol* symbols;

		~RCXFile()
		{
			delete[] chunks;
			delete[] symbols;
		}
	};
#pragma pack(pop)
#define FILE_HEADER_LENGTH 12

	bool ParseRCXFile(const char* filePath, RCXFile& file);
	bool DownloadProgram(const char* filePath, BYTE programSlot, Tower::RequestData* towerData);
	
	BYTE GetValueFromPair(const BYTE a, const BYTE b);
	bool DownloadFirmware(const char* filePath, Tower::RequestData* towerData);
}