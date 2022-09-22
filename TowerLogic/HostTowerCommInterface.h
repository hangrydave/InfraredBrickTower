#pragma once

#if defined(WIN64)

#include <Windows.h>

#elif defined(__linux)

#define BYTE unsigned char
#define WORD unsigned short
#define UINT unsigned int
#define BOOL bool
#define VOID void
#define ULONG unsigned long
#define USHORT unsigned short

#endif

#define TOWER_READ_PIPE_ID 129
#define TOWER_WRITE_PIPE_ID 2

class HostTowerCommInterface
{
public:
	virtual BOOL ControlTransfer(
		BYTE request,
		WORD value,
		WORD index,
		USHORT bufferLength,
		BYTE* buffer,
		ULONG& lengthTransferred) const = 0;
	
	virtual BOOL Write(
		BYTE* buffer,
		ULONG bufferLength,
		ULONG& lengthWritten) const = 0;

	virtual BOOL Read(
		BYTE* buffer,
		ULONG bufferLength,
		ULONG& lengthRead) const = 0;

	virtual BOOL Flush() const = 0;
};