#include "pch.h"
#include "TowerController.h"
#include <stdlib.h>
#include <assert.h>

#if defined(WIN64)
#elif defined(__linux)
#include <unistd.h>
#endif

#define BYTE unsigned char
#define WORD unsigned short

#define MAX_WRITE_ATTEMPTS 5
#define MAX_READ_ATTEMPTS 5

// This number isn't pulled from documentation or anything, I've just found I need a small pause between things for it to work, and this is reasonably small
#define WRITE_PAUSE_TIME 150

namespace Tower
{
	bool ReadData(
		BYTE* buffer,
		unsigned long bufferLength,
		unsigned long& lengthRead,
		RequestData* data)
	{
		lengthRead = 0;

		int readAttemptCount = 0;
		bool success = false;
		while (readAttemptCount < MAX_READ_ATTEMPTS &&
			   lengthRead == 0)
		{
			printf("Read zero bytes, trying again...\n");
			success = data->commInterface->Read(buffer, bufferLength, lengthRead);

			if (lengthRead == 1)
			{
				success = data->commInterface->Read(buffer + 1, bufferLength, lengthRead);	
				lengthRead++;

			}
			readAttemptCount++;
		}

		if (readAttemptCount >= MAX_READ_ATTEMPTS)
		{
			printf("Failed to read data three times in a row.\n");
		}

		return success || (lengthRead > 0);
	}

	bool WriteData(
		BYTE* buffer,
		unsigned long bufferLength,
		unsigned long& lengthWritten,
		RequestData* data,
		bool preFlush)
	{
		if (preFlush)
		{
			// nqc flushes the read before writing, which is a very good idea!
			unsigned long lengthRead = -1;
			BYTE* readBuffer = new BYTE[512];

			while (lengthRead != 0)
			{
				data->commInterface->Read(readBuffer, 512, lengthRead);
			}

			delete[] readBuffer;
		}

		// Try to write the data!
		int writeAttemptCount = 0;
		bool success = false;
		while (!success && writeAttemptCount < MAX_WRITE_ATTEMPTS)
		{
			success = data->commInterface->Write(buffer, bufferLength, lengthWritten);
			// (This number was chosen arbitrarily)
			// What I think is happening is that when downloading firmware, it works fine because
			// the packets are big enough to provide a timeout long enough, but if the data is smaller,
			// we need an additional timeout.
			if (bufferLength < 350)
			{
				// give time to finish
#if defined(WIN64)
            	Sleep(WRITE_PAUSE_TIME);
#elif defined(__linux)
            	sleep(WRITE_PAUSE_TIME / 1000);
#endif
			}
			
			writeAttemptCount++;
		}

		return bufferLength == lengthWritten;
	}

	void Flush(CommBuffer buffer, RequestData* data)
	{
		BYTE loByte = (BYTE)buffer;
		BYTE hiByte = 0;
		MakeRequest(
			RequestType::FLUSH,
			loByte,
			hiByte,
			data);
	}

	void Reset(RequestData* data)
	{
		MakeRequest(RequestType::RESET, data);
	}

	Power GetPower(RequestData* data)
	{
		BYTE loByte = 0;
		BYTE hiByte = 0;
		MakeRequest(
			RequestType::GET_POWER,
			loByte,
			hiByte,
			data);

		return (Power) *(data->replyBuffer + 4);
	}

	StatisticsData GetStatistics(RequestData* data)
	{
		MakeRequest(RequestType::GET_STATISTICS, data);

		return *reinterpret_cast<StatisticsData*>(data->replyBuffer + 4);
	}

	void ResetStatistics(RequestData* data)
	{
		MakeRequest(RequestType::RESET_STATISTICS, data);
	}

	CommSpeed GetTransmissionSpeed(RequestData* data)
	{
		MakeRequest(RequestType::GET_TRANSMISSION_SPEED, data);

		return (CommSpeed) *(data->replyBuffer + 4);
	}

	void SetTransmissionSpeed(CommSpeed speed, RequestData* data)
	{
		MakeRequest(
			RequestType::SET_TRANSMISSION_SPEED,
			(BYTE)speed,
			0,
			data);
	}

	CommSpeed GetReceivingSpeed(RequestData* data)
	{
		MakeRequest(RequestType::GET_RECEIVING_SPEED, data);

		return (CommSpeed) *(data->replyBuffer + 4);
	}

	void SetReceivingSpeed(CommSpeed speed, RequestData* data)
	{
		MakeRequest(
			RequestType::SET_RECEIVING_SPEED,
			(BYTE)speed,
			0,
			data);
	}

	TransmitterState GetTransmitterState(RequestData* data)
	{
		MakeRequest(RequestType::GET_TRANSMITTER_STATE, data);

		return (TransmitterState) *(data->replyBuffer + 4);
	}

	LEDColor GetLEDColor(LED led, RequestData* data)
	{
		BYTE loByte = (BYTE)led;
		BYTE hiByte = 0;
		MakeRequest(
			RequestType::GET_LED,
			loByte,
			hiByte,
			data);

		return (LEDColor) *(data->replyBuffer + 5);
	}

	void SetLEDColor(LED led, LEDColor color, RequestData* data)
	{
		BYTE loByte = (BYTE)led;
		BYTE hiByte = (BYTE)color;
		MakeRequest(
			RequestType::SET_LED,
			loByte,
			hiByte,
			data);
	}

	CapabilitiesData GetCapabilities(CapabilityLink link, RequestData* data)
	{
		MakeRequest(RequestType::GET_CAPABILITIES, (WORD)link, data);

		return *reinterpret_cast<CapabilitiesData*>(data->replyBuffer + 4);
	}

	VersionData GetVersion(RequestData* data)
	{
		MakeRequest(RequestType::GET_VERSION, data);

		return *reinterpret_cast<VersionData*>(data->replyBuffer + 4);
	}

	void GetCopyright(RequestData* data)
	{
		MakeRequest(RequestType::GET_COPYRIGHT, data);

		ReadStringFromReplyBuffer(data);
	}

	void GetCredits(RequestData* data)
	{
		MakeRequest(RequestType::GET_CREDITS, data);

		ReadStringFromReplyBuffer(data);
	}

	void ReadStringFromReplyBuffer(RequestData* data)
	{
		// the vendor requests that reply with a string put the length at the front of the buffer
		data->stringLength = *((WORD*)(data->replyBuffer));

		// start at 4; skip the non-string stuff
		for (int i = 4; i < data->stringLength; i += 2)
		{
			data->stringBuffer[(i - 4) / 2] = data->replyBuffer[i];

			wchar_t wide = (wchar_t) data->replyBuffer[i];
			data->wideStringBuffer[(i - 4) / 2] = wide;
		}
	}

	void SetParameter(
		ParamType parameter,
		BYTE value,
		RequestData* data)
	{
		MakeRequest(
			RequestType::SET_PARAMETER,
			(BYTE)parameter,
			value,
			data);
	}

	BYTE GetParameter(ParamType parameter, RequestData* data)
	{
		MakeRequest(
			RequestType::GET_PARAMETER,
			(BYTE)parameter,
			0,
			data);

		return *(data->replyBuffer + 3);
	}

	void MakeRequest(RequestType request, RequestData* data)
	{
		MakeRequest(request, 0, data);
	}

	void MakeRequest(
		RequestType request,
		BYTE loByte,
		BYTE hiByte,
		RequestData* data)
	{
		WORD value = ((hiByte << 8) | loByte);
		MakeRequest(request, value, data);
	}

	void MakeRequest(
		RequestType request,
		WORD value,
		RequestData* data)
	{
		// not used rn, will implement when used
		WORD index = 0;

		bool success = data->commInterface->ControlTransfer(
			(BYTE)request,
			value,
			index,
			REQUEST_REPLY_BUFFER_LENGTH,
			data->replyBuffer,
			data->lastReplyLength);

		if (!success)
		{
			data->lastRequestError = RequestError::BAD_PARAMETER;
		}
		else
		{
			BYTE errorByte = *(data->replyBuffer + 2);
			data->lastRequestError = (RequestError)errorByte;
		}

		if (data->lastRequestError != RequestError::SUCCESS)
		{
#if DEBUG == 1
			__debugbreak();
#endif

			printf("Tower request error: ");
			switch (data->lastRequestError)
			{
			case RequestError::BAD_PARAMETER:
				printf("bad param");
				break;
			case RequestError::BUSY:
				printf("busy");
				break;
			case RequestError::NOT_ENOUGH_POWER:
				printf("not enough power");
				break;
			case RequestError::WRONG_MODE:
				printf("wrong mode");
				break;
			case RequestError::INTERNAL_ERROR:
				printf("internal error");
				break;
			case RequestError::BAD_REQUEST:
				printf("bad request");
				break;
			}
			printf("\n");
		}
	}
}
