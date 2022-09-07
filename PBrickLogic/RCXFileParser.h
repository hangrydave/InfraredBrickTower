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

	inline VOID ParseFile()
	{
		std::ifstream input("beep.rcx", std::ios::binary);
		if (!input)
			printf("die");
		
		RCXFile file;
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
	}
}