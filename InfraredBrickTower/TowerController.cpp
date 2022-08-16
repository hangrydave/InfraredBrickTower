#include "TowerController.h"
#include "LegoHeaders/LegoVendReq.h"
#include <string>

#define MAX_WRITE_ATTEMPTS 3
#define MAX_READ_ATTEMPTS 3

TowerController::TowerController(const HostTowerCommInterface* usbInterface)
{
	this->usbInterface = usbInterface;

	this->readAttemptCount = 0;
	this->writeAttemptCount = 0;

	this->lastRequestError = TowerRequestError::SUCCESS;
	this->lastReplyLength = 0;

	this->replyBufferSize = 1000;
	this->replyBuffer = new BYTE[this->replyBufferSize];
}

TowerController::~TowerController()
{
	delete this->replyBuffer;
}

VOID TowerController::ReadData(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthRead)
{
	printf("READ\n");

	TowerTransmitterState transmitterState = this->GetTransmitterState();
	TowerErrorStatus errorStatus = this->GetErrorStatus();
	TowerRequestError requestError = this->GetLastRequestError();
	while (transmitterState == TowerTransmitterState::BUSY) { printf("Tower busy...\n"); transmitterState = this->GetTransmitterState(); }
	while (errorStatus != TowerErrorStatus::OK) { printf("Tower error status: %d\n", errorStatus); errorStatus = this->GetErrorStatus(); }
	while (requestError != TowerRequestError::SUCCESS) { printf("Tower request error: %d\n", requestError); requestError = this->GetLastRequestError(); }

	this->readAttemptCount = 0;
	BOOL success = FALSE;
	while (!success && this->readAttemptCount < MAX_READ_ATTEMPTS)
	{
		success = this->usbInterface->Read(buffer, bufferLength, lengthRead);

		if (lengthRead == 1)
		{
			// this happens after the thing is plugged in. there are better ways to handle this, i'm sure, but... ez pz
			success = this->usbInterface->Read(buffer + 1, bufferLength, lengthRead);
			lengthRead++;
		}

		this->readAttemptCount++;
	}
}

VOID TowerController::WriteData(
	PUCHAR buffer,
	ULONG bufferLength,
	ULONG& lengthWritten)
{
	printf("WRITE\n");

	TowerTransmitterState transmitterState = this->GetTransmitterState();
	TowerErrorStatus errorStatus = this->GetErrorStatus();
	TowerRequestError requestError = this->GetLastRequestError();
	while (transmitterState == TowerTransmitterState::BUSY) { printf("Tower busy...\n"); transmitterState = this->GetTransmitterState(); }
	while (errorStatus != TowerErrorStatus::OK) { printf("Tower error status: %d\n", errorStatus); errorStatus = this->GetErrorStatus(); }
	while (requestError != TowerRequestError::SUCCESS) { printf("Tower request error: %d\n", requestError); requestError = this->GetLastRequestError(); }

	this->writeAttemptCount = 0;
	BOOL success = FALSE;
	while (!success && this->writeAttemptCount < MAX_WRITE_ATTEMPTS)
	{
		success = this->usbInterface->Write(buffer, bufferLength, lengthWritten);
		Sleep(150); // give time to finish
		this->writeAttemptCount++;
	}

}

TowerRequestError TowerController::GetLastRequestError()
{
	return lastRequestError;
}

VOID TowerController::Flush(TowerBuffer buffer)
{
	BYTE loByte = (BYTE)buffer;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::FLUSH,
		loByte,
		hiByte);
}

VOID TowerController::Reset()
{
	MakeRequest(TowerRequestType::RESET);
}

TowerPower TowerController::GetPower()
{
	BYTE loByte = 0;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::GET_POWER,
		loByte,
		hiByte);

	return (TowerPower) *(this->replyBuffer + 4);
}

TowerStatData TowerController::GetStatistics()
{
	MakeRequest(TowerRequestType::GET_STATISTICS);

	return *reinterpret_cast<TowerStatData*>(this->replyBuffer + 4);
}

VOID TowerController::ResetStatistics()
{
	MakeRequest(TowerRequestType::RESET_STATISTICS);
}

TowerCommSpeed TowerController::GetTransmissionSpeed()
{
	MakeRequest(TowerRequestType::GET_TRANSMISSION_SPEED);

	return (TowerCommSpeed) *(this->replyBuffer + 4);
}

VOID TowerController::SetTransmissionSpeed(TowerCommSpeed speed)
{
	MakeRequest(
		TowerRequestType::SET_TRANSMISSION_SPEED,
		(BYTE)speed,
		0);
}

TowerCommSpeed TowerController::GetReceivingSpeed()
{
	MakeRequest(TowerRequestType::GET_RECEIVING_SPEED);

	return (TowerCommSpeed) *(this->replyBuffer + 4);
}

VOID TowerController::SetReceivingSpeed(TowerCommSpeed speed)
{
	MakeRequest(
		TowerRequestType::SET_RECEIVING_SPEED,
		(BYTE)speed,
		0);
}

TowerTransmitterState TowerController::GetTransmitterState()
{
	MakeRequest(TowerRequestType::GET_TRANSMITTER_STATE);

	return (TowerTransmitterState) *(this->replyBuffer + 4);
}

TowerLEDColor TowerController::GetLEDColor(TowerLED led)
{
	BYTE loByte = (BYTE)led;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::GET_LED,
		loByte,
		hiByte);

	return (TowerLEDColor) *(this->replyBuffer + 5);
}

VOID TowerController::SetLEDColor(TowerLED led, TowerLEDColor color)
{
	BYTE loByte = (BYTE)led;
	BYTE hiByte = (BYTE)color;
	MakeRequest(
		TowerRequestType::SET_LED,
		loByte,
		hiByte);
}

TowerCapabilitiesData TowerController::GetCapabilities(TowerCapabilityLink link)
{
	MakeRequest(TowerRequestType::GET_CAPABILITIES, (WORD)link);

	return *reinterpret_cast<TowerCapabilitiesData*>(this->replyBuffer + 4);
}

TowerVersionData TowerController::GetVersion()
{
	MakeRequest(TowerRequestType::GET_VERSION);

	return *reinterpret_cast<TowerVersionData*>(this->replyBuffer + 4);
}

VOID TowerController::GetCopyright(CHAR*& buffer, INT& length)
{
	MakeRequest(TowerRequestType::GET_COPYRIGHT);

	ReadStringFromReplyBuffer(buffer, length);
}

VOID TowerController::GetCredits(CHAR*& buffer, INT& length)
{
	MakeRequest(TowerRequestType::GET_CREDITS);

	ReadStringFromReplyBuffer(buffer, length);
}

VOID TowerController::ReadStringFromReplyBuffer(CHAR*& buffer, INT& length)
{
	// the vendor requests that reply with a string put the length at the front of the buffer
	UINT stringLength = *((WORD*)(this->replyBuffer));
	stringLength--; // the last character is garbage

	// the string is formatted "L I K E   T H I S" so we need to fix that

	length = 0;
	buffer = new CHAR[stringLength / 2];

	// start at 4; skip the non-string stuff
	for (UINT i = 4; i < stringLength; i++)
	{
		char c = this->replyBuffer[i];

		if (c == '\0' && i < this->lastReplyLength - 1)
		{
			char prev = this->replyBuffer[i - 1];
			char next = this->replyBuffer[i + 1];

			if (prev == '\0' && next == '\0')
			{
				buffer[length++] = c;
			}
		}
		else
		{
			buffer[length++] = c;
		}
	}
}

VOID TowerController::SetParameter(
	TowerParamType parameter,
	BYTE value)
{
	MakeRequest(
		TowerRequestType::SET_PARAMETER,
		(BYTE)parameter,
		value);
}

BYTE TowerController::GetParameter(TowerParamType parameter)
{
	MakeRequest(
		TowerRequestType::GET_PARAMETER,
		(BYTE)parameter,
		0);

	return *(this->replyBuffer + 3);
}

VOID TowerController::MakeRequest(TowerRequestType request)
{
	MakeRequest(request, 0);
}

VOID TowerController::MakeRequest(
	TowerRequestType request,
	BYTE loByte,
	BYTE hiByte)
{
	WORD value = ((hiByte << 8) | loByte);
	MakeRequest(request, value);
}

VOID TowerController::MakeRequest(
	TowerRequestType request,
	WORD value)
{
	// not used rn, will implement when used
	WORD index = 0;

	BOOL success = usbInterface->ControlTransfer(
		(BYTE)request,
		value,
		index,
		this->replyBufferSize,
		this->replyBuffer,
		this->lastReplyLength);

	if (!success)
	{
		this->lastRequestError = TowerRequestError::BAD_PARAMETER;
	}
	else
	{
		BYTE errorByte = *(this->replyBuffer + 2);
		this->lastRequestError = (TowerRequestError)errorByte;
	}
}
