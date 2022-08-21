#include "pch.h"
#include "TowerController.h"
#include <string>
#include <stdlib.h>

#define MAX_WRITE_ATTEMPTS 3
#define MAX_READ_ATTEMPTS 3

// This number isn't pulled from documentation or anything, I've just found I need a small pause between things for it to work, and this is reasonably small
#define WRITE_PAUSE_TIME 150

namespace Tower
{
	VOID ReadData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthRead,
		TowerData* data)
	{
		while (GetTransmitterState(data) == TowerTransmitterState::BUSY) { printf("Tower busy, can't read...\n"); }

		INT readAttemptCount = 0;
		BOOL success = FALSE;
		while (!success && readAttemptCount < MAX_READ_ATTEMPTS)
		{
			success = data->commInterface->Read(buffer, bufferLength, lengthRead);

			if (lengthRead == 1)
			{
				// this happens after the thing is plugged in. there are better ways to handle this, i'm sure, but... ez pz
				success = data->commInterface->Read(buffer + 1, bufferLength, lengthRead);
				lengthRead++;
			}

			readAttemptCount++;
		}
	}

	VOID WriteData(
		PUCHAR buffer,
		ULONG bufferLength,
		ULONG& lengthWritten,
		TowerData* data)
	{
		while (GetTransmitterState(data) == TowerTransmitterState::BUSY) { printf("Tower busy, can't write...\n"); }

		INT writeAttemptCount = 0;
		BOOL success = FALSE;
		while (!success && writeAttemptCount < MAX_WRITE_ATTEMPTS)
		{
			success = data->commInterface->Write(buffer, bufferLength, lengthWritten);
			Sleep(WRITE_PAUSE_TIME); // give time to finish
			writeAttemptCount++;
		}

	}

	VOID Flush(TowerBuffer buffer, TowerData* data)
	{
		BYTE loByte = (BYTE)buffer;
		BYTE hiByte = 0;
		MakeRequest(
			TowerRequestType::FLUSH,
			loByte,
			hiByte,
			data);
	}

	VOID Reset(TowerData* data)
	{
		MakeRequest(TowerRequestType::RESET, data);
	}

	TowerPower GetPower(TowerData* data)
	{
		BYTE loByte = 0;
		BYTE hiByte = 0;
		MakeRequest(
			TowerRequestType::GET_POWER,
			loByte,
			hiByte,
			data);

		return (TowerPower) *(data->replyBuffer + 4);
	}

	TowerStatData GetStatistics(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_STATISTICS, data);

		return *reinterpret_cast<TowerStatData*>(data->replyBuffer + 4);
	}

	VOID ResetStatistics(TowerData* data)
	{
		MakeRequest(TowerRequestType::RESET_STATISTICS, data);
	}

	TowerCommSpeed GetTransmissionSpeed(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_TRANSMISSION_SPEED, data);

		return (TowerCommSpeed) *(data->replyBuffer + 4);
	}

	VOID SetTransmissionSpeed(TowerCommSpeed speed, TowerData* data)
	{
		MakeRequest(
			TowerRequestType::SET_TRANSMISSION_SPEED,
			(BYTE)speed,
			0,
			data);
	}

	TowerCommSpeed GetReceivingSpeed(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_RECEIVING_SPEED, data);

		return (TowerCommSpeed) *(data->replyBuffer + 4);
	}

	VOID SetReceivingSpeed(TowerCommSpeed speed, TowerData* data)
	{
		MakeRequest(
			TowerRequestType::SET_RECEIVING_SPEED,
			(BYTE)speed,
			0,
			data);
	}

	TowerTransmitterState GetTransmitterState(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_TRANSMITTER_STATE, data);

		return (TowerTransmitterState) *(data->replyBuffer + 4);
	}

	TowerLEDColor GetLEDColor(TowerLED led, TowerData* data)
	{
		BYTE loByte = (BYTE)led;
		BYTE hiByte = 0;
		MakeRequest(
			TowerRequestType::GET_LED,
			loByte,
			hiByte,
			data);

		return (TowerLEDColor) *(data->replyBuffer + 5);
	}

	VOID SetLEDColor(TowerLED led, TowerLEDColor color, TowerData* data)
	{
		BYTE loByte = (BYTE)led;
		BYTE hiByte = (BYTE)color;
		MakeRequest(
			TowerRequestType::SET_LED,
			loByte,
			hiByte,
			data);
	}

	TowerCapabilitiesData GetCapabilities(TowerCapabilityLink link, TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_CAPABILITIES, (WORD)link, data);

		return *reinterpret_cast<TowerCapabilitiesData*>(data->replyBuffer + 4);
	}

	TowerVersionData GetVersion(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_VERSION, data);

		return *reinterpret_cast<TowerVersionData*>(data->replyBuffer + 4);
	}

	VOID GetCopyright(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_COPYRIGHT, data);

		ReadStringFromReplyBuffer(data);
	}

	VOID GetCredits(TowerData* data)
	{
		MakeRequest(TowerRequestType::GET_CREDITS, data);

		ReadStringFromReplyBuffer(data);
	}

	VOID ReadStringFromReplyBuffer(TowerData* data)
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
		TowerParamType parameter,
		BYTE value,
		TowerData* data)
	{
		MakeRequest(
			TowerRequestType::SET_PARAMETER,
			(BYTE)parameter,
			value,
			data);
	}

	BYTE GetParameter(TowerParamType parameter, TowerData* data)
	{
		MakeRequest(
			TowerRequestType::GET_PARAMETER,
			(BYTE)parameter,
			0,
			data);

		return *(data->replyBuffer + 3);
	}

	VOID MakeRequest(TowerRequestType request, TowerData* data)
	{
		MakeRequest(request, 0, data);
	}

	VOID MakeRequest(
		TowerRequestType request,
		BYTE loByte,
		BYTE hiByte,
		TowerData* data)
	{
		WORD value = ((hiByte << 8) | loByte);
		MakeRequest(request, value, data);
	}

	VOID MakeRequest(
		TowerRequestType request,
		WORD value,
		TowerData* data)
	{
		// not used rn, will implement when used
		WORD index = 0;

		BOOL success = data->commInterface->ControlTransfer(
			(BYTE)request,
			value,
			index,
			REPLY_BUFFER_LENGTH,
			data->replyBuffer,
			data->lastReplyLength);

		if (!success)
		{
			data->lastRequestError = TowerRequestError::BAD_PARAMETER;
		}
		else
		{
			BYTE errorByte = *(data->replyBuffer + 2);
			data->lastRequestError = (TowerRequestError)errorByte;
		}

		if (data->lastRequestError != TowerRequestError::SUCCESS)
		{
#if DEBUG == 1
			__debugbreak();
#endif

			printf("Tower request error: ");
			switch (data->lastRequestError)
			{
			case TowerRequestError::BAD_PARAMETER:
				printf("bad param");
				break;
			case TowerRequestError::BUSY:
				printf("busy");
				break;
			case TowerRequestError::NOT_ENOUGH_POWER:
				printf("not enough power");
				break;
			case TowerRequestError::WRONG_MODE:
				printf("wrong mode");
				break;
			case TowerRequestError::INTERNAL_ERROR:
				printf("internal error");
				break;
			case TowerRequestError::BAD_REQUEST:
				printf("bad request");
				break;
			}
			printf("\n");
		}
	}
}
