#include "pch.h"
#include "TowerController.h"
#include <stdlib.h>

#define MAX_WRITE_ATTEMPTS 3
#define MAX_READ_ATTEMPTS 3

// This number isn't pulled from documentation or anything, I've just found I need a small pause between things for it to work, and this is reasonably small
#define WRITE_PAUSE_TIME 150

namespace Tower
{
	BOOL SendData(
		PUCHAR inputBuffer,
		ULONG inputBufferLength,
		PUCHAR replyBuffer,
		ULONG replyBufferLength,
		ULONG& lengthRead,
		RequestData* data)
	{
		BOOL writeSuccess = WriteData(inputBuffer, inputBufferLength, data);
		if (!writeSuccess)
			return FALSE;

		BOOL readSuccess = ReadData(replyBuffer, replyBufferLength, lengthRead, data);
		return readSuccess;
	}

	BOOL ReadData(PUCHAR buffer, ULONG bufferLength, RequestData* data, BOOL skipSingleByteBefore)
	{
		ULONG lengthRead;
		return ReadData(buffer, bufferLength, lengthRead, data, skipSingleByteBefore);
	}

	BOOL ReadData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead,
		RequestData* data,
		BOOL skipSingleByteBefore)
	{
		while (GetTransmitterState(data) == TransmitterState::BUSY) { printf("Tower busy, can't read...\n"); }

		lengthRead = 0;

		INT readAttemptCount = 0;
		BOOL success = FALSE;
		while (!success && readAttemptCount < MAX_READ_ATTEMPTS)
		{
			success = data->commInterface->Read(buffer, bufferLength, lengthRead);

			if (!skipSingleByteBefore)
			{
				break;
			}

			if (lengthRead == 1)
			{
				// this happens after the thing is plugged in.
				// maybe are better ways to handle this, i'm sure, but... ez pz

				// UPDATE: this happens EVERY TIME... no idea why?
				success = data->commInterface->Read(buffer, bufferLength, lengthRead);
				lengthRead++;

			}
			readAttemptCount++;
		}

		return success;
	}

	BOOL WriteData(PUCHAR buffer, ULONG bufferLength, RequestData* data)
	{
		ULONG lengthWritten;
		return WriteData(buffer, bufferLength, lengthWritten, data);
	}

	BOOL WriteData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten,
		RequestData* data)
	{
		while (GetTransmitterState(data) == TransmitterState::BUSY) { printf("Tower busy, can't write...\n"); }

		INT writeAttemptCount = 0;
		BOOL success = FALSE;
		while (!success && writeAttemptCount < MAX_WRITE_ATTEMPTS)
		{
			success = data->commInterface->Write(buffer, bufferLength, lengthWritten);
			Sleep(WRITE_PAUSE_TIME); // give time to finish
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
