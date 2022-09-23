#pragma once

#if defined(WIN64)

#include <Windows.h>

#elif defined(__linux)

#define BYTE unsigned char
#define WORD unsigned short

#endif

#define TOWER_READ_PIPE_ID 129
#define TOWER_WRITE_PIPE_ID 2

class HostTowerCommInterface
{
public:
    virtual ~HostTowerCommInterface() {}

	virtual bool ControlTransfer(
		BYTE request,
		WORD value,
		unsigned short index,
		unsigned short bufferLength,
		BYTE* buffer,
		unsigned long& lengthTransferred) const = 0;
	
	virtual bool Write(
		BYTE* buffer,
		unsigned long bufferLength,
		unsigned long& lengthWritten) const = 0;

	virtual bool Read(
		BYTE* buffer,
		unsigned long bufferLength,
		unsigned long& lengthRead) const = 0;

	virtual bool Flush() const = 0;

    virtual bool Close() const = 0;
};