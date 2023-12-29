#include "pch.h"
#include "TowerController.h"
#include <stdlib.h>
#include <assert.h>

#define MAX_WRITE_ATTEMPTS 4
#define MAX_READ_ATTEMPTS 4

// This number isn't pulled from documentation or anything, I've just found I need a small pause between things for it to work, and this is reasonably small
#define WRITE_PAUSE_TIME 150

namespace Tower
{
	BOOL ReadData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead,
		RequestData* data)
	{
		lengthRead = 0;

		INT readAttemptCount = 0;
		BOOL success = FALSE;
		while (!success &&
			readAttemptCount < MAX_READ_ATTEMPTS &&
			lengthRead == 0)
		{
			success = data->commInterface->Read(buffer, bufferLength, lengthRead);

			if (lengthRead == 1)
			{
				success = data->commInterface->Read(buffer + 1, bufferLength, lengthRead);	
				lengthRead++;

			}
			readAttemptCount++;
		}

		return success || (lengthRead > 0);
	}

	BOOL WriteData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten,
		RequestData* data,
		BOOL preFlush)
	{
		if (preFlush)
		{
			// nqc flushes the read before writing, which is a very good idea!
			ULONG lengthRead = -1;
			PUCHAR readBuffer = new UCHAR[512];

			while (lengthRead != 0)
			{
				data->commInterface->Read(readBuffer, 512, lengthRead);
			}

			delete[] readBuffer;
		}

		// Try to write the data!
		INT writeAttemptCount = 0;
		BOOL success = FALSE;
		while (!success && writeAttemptCount < MAX_WRITE_ATTEMPTS)
		{
			success = data->commInterface->Write(buffer, bufferLength, lengthWritten);
			// (This number was chosen arbitrarily)
			// What I think is happening is that when downloading firmware, it works fine because
			// the packets are big enough to provide a timeout long enough, but if the data is smaller,
			// we need an additional timeout.
			if (bufferLength < 350)
			{
				Sleep(WRITE_PAUSE_TIME); // give time to finish
			}
			
			writeAttemptCount++;
		}

		return bufferLength == lengthWritten;
	}

	VOID Flush(CommBuffer buffer, RequestData* data)
	{
		BYTE loByte = (BYTE)buffer;
		BYTE hiByte = 0;
		MakeRequest(
			RequestType::FLUSH,
			loByte,
			hiByte,
			data);
	}

	VOID Reset(RequestData* data)
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

	VOID ResetStatistics(RequestData* data)
	{
		MakeRequest(RequestType::RESET_STATISTICS, data);
	}

	CommSpeed GetTransmissionSpeed(RequestData* data)
	{
		MakeRequest(RequestType::GET_TRANSMISSION_SPEED, data);

		return (CommSpeed) *(data->replyBuffer + 4);
	}

	VOID SetTransmissionSpeed(CommSpeed speed, RequestData* data)
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

	VOID SetReceivingSpeed(CommSpeed speed, RequestData* data)
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

	VOID SetLEDColor(LED led, LEDColor color, RequestData* data)
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

	VOID GetCopyright(RequestData* data)
	{
		MakeRequest(RequestType::GET_COPYRIGHT, data);

		ReadStringFromReplyBuffer(data);
	}

	VOID GetCredits(RequestData* data)
	{
		MakeRequest(RequestType::GET_CREDITS, data);

		ReadStringFromReplyBuffer(data);
	}

	VOID ReadStringFromReplyBuffer(RequestData* data)
	{
		// the vendor requests that reply with a string put the length at the front of the buffer
		data->stringLength = *((WORD*)(data->replyBuffer));

		// start at 4; skip the non-string stuff
		for (UINT i = 4; i < data->stringLength; i += 2)
		{
			WCHAR wide = (WCHAR) data->replyBuffer[i];
			data->stringBuffer[(i - 4) / 2] = wide;
		}
	}

	VOID SetParameter(
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

	VOID MakeRequest(RequestType request, RequestData* data)
	{
		MakeRequest(request, 0, data);
	}

	VOID MakeRequest(
		RequestType request,
		BYTE loByte,
		BYTE hiByte,
		RequestData* data)
	{
		WORD value = ((hiByte << 8) | loByte);
		MakeRequest(request, value, data);
	}

	VOID MakeRequest(
		RequestType request,
		WORD value,
		RequestData* data)
	{
		// not used rn, will implement when used
		WORD index = 0;

		BOOL success = data->commInterface->ControlTransfer(
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
