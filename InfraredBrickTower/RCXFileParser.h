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

namespace RCXParser
{
#pragma pack(push, 1)
	struct FileHeader
	{
		UINT32 signature;
		UINT16 version;
		UINT16 chunkCount;
		UINT16 symbolCount;
		BYTE targetType;
		BYTE reserved;
	};
#pragma pack(pop)
#define FILE_HEADER_LENGTH 12

#pragma pack(push, 1)
	struct Chunk
	{
		BYTE type;
		BYTE number;
		WORD length;

		BYTE* data; // this goes beyond the bound of CHUNK_HEADER_LENGTH
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
#define SYMBOL_LENGTH 4

	inline VOID ParseFile()
	{
		std::ifstream input("beep.rcx", std::ios::binary);
		if (!input)
			printf("die");
		
		FileHeader header;
		input.read(reinterpret_cast<char*>(&header), FILE_HEADER_LENGTH);

		Chunk* chunks = new Chunk[header.chunkCount];
		for (UINT i = 0; i < header.chunkCount; i++)
		{
			input.read(reinterpret_cast<char*>(&chunks[i]), CHUNK_HEADER_LENGTH);

			WORD chunkLength = chunks[i].length;
			chunks[i].data = new BYTE[chunkLength];
			input.read(reinterpret_cast<char*>(chunks[i].data), chunkLength);

			// from line 100 in rcxifile.h in the nqc project
			// also line 240 in RCX_Image.cpp
			UINT paddingBytes = (4 - chunkLength) & 3;
			input.seekg(paddingBytes, std::ios::cur);
		}

		Symbol* symbols = new Symbol[header.symbolCount];
		for (UINT i = 0; i < header.symbolCount; i++)
		{
			input.read(reinterpret_cast<char*>(&symbols[i]), SYMBOL_LENGTH);
			symbols[i].name = new CHAR[256];
			input.read(symbols[i].name, symbols[i].length);

			// TODO: look at logic at line 261 onwards in RCX_Image.cpp
			// sometimes there may not be a symbol, so just do a seek instead
		}

		input.close();
	}
}