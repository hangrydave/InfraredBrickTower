#pragma once

#include <fstream>
#include <Windows.h>

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
#pragma pack(push, 1)
	struct Chunk
	{
		BYTE type;
		BYTE number;
		WORD length;

		BYTE* data;
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

		CHAR* name;
	};
#pragma pack(pop)
#define SYMBOL_HEADER_LENGTH 4

#pragma pack(push, 1)
	struct RCXFile
	{
		UINT32 signature;
		UINT16 version;
		UINT16 chunkCount;
		UINT16 symbolCount;
		BYTE targetType;
		BYTE reserved;

		Chunk* chunks;
		Symbol* symbols;
	};
#pragma pack(pop)
#define FILE_HEADER_LENGTH 12

	VOID ParseFile(const char* fileName, RCXFile& file);
}