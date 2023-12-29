#pragma once

#include <Windows.h>

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

	virtual BOOL EnableForeverTimeout() const = 0;

	virtual BOOL ResetTimeout() const = 0;
	
	virtual BOOL Write(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten) const = 0;

	virtual BOOL Read(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead) const = 0;

	virtual BOOL Flush() const = 0;
};