#pragma once

#define TOWER_READ_PIPE_ID 129
#define TOWER_WRITE_PIPE_ID 2

class HostTowerCommInterface
{
public:
	virtual bool ControlTransfer(
		unsigned char request,
		unsigned short value,
		unsigned short index,
		unsigned short bufferLength,
		unsigned char* buffer,
		unsigned long& lengthTransferred) const = 0;

	/*virtual BOOL EnableForeverTimeout() const = 0;

	virtual BOOL ResetTimeout() const = 0;*/
	
	virtual bool Write(
		unsigned char* buffer,
		unsigned long bufferLength,
		unsigned long& lengthWritten) const = 0;

	virtual bool Read(
		unsigned char* buffer,
		unsigned long bufferLength,
		unsigned long& lengthRead) = 0;

	virtual bool Flush() const = 0;

	virtual bool Close() const = 0;
};