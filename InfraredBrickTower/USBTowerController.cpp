#include "usbtowercontroller.h"
#include "LegoHeaders/LegoVendReq.h"
#include <string>

USBTowerController::USBTowerController(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;

	this->lastRequestError = TowerRequestError::SUCCESS;
	this->lastReplyLength = 0;

	this->replyBufferSize = 1000;
	this->replyBuffer = new BYTE[this->replyBufferSize];
}

USBTowerController::~USBTowerController()
{
	delete this->replyBuffer;
}

TowerRequestError USBTowerController::GetLastRequestError()
{
	return lastRequestError;
}

VOID USBTowerController::Flush(TowerBuffer buffer)
{
	BYTE loByte = (BYTE)buffer;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::FLUSH,
		loByte,
		hiByte);
}

VOID USBTowerController::Reset()
{
	MakeRequest(TowerRequestType::RESET);
}

TowerPower USBTowerController::GetPower()
{
	BYTE loByte = 0;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::GET_POWER,
		loByte,
		hiByte);

	return (TowerPower) * (this->replyBuffer + 4);
}

TowerStatData USBTowerController::GetStatistics()
{
	MakeRequest(TowerRequestType::GET_STATISTICS);

	return *reinterpret_cast<TowerStatData*>(this->replyBuffer + 4);
}

VOID USBTowerController::ResetStatistics()
{
	MakeRequest(TowerRequestType::RESET_STATISTICS);
}

TowerCommSpeed USBTowerController::GetTransmissionSpeed()
{
	MakeRequest(TowerRequestType::GET_TRANSMISSION_SPEED);

	return (TowerCommSpeed) *(this->replyBuffer + 4);
}

VOID USBTowerController::SetTransmissionSpeed(TowerCommSpeed speed)
{
	MakeRequest(
		TowerRequestType::SET_TRANSMISSION_SPEED,
		(BYTE)speed,
		0);
}

TowerCommSpeed USBTowerController::GetReceivingSpeed()
{
	MakeRequest(TowerRequestType::GET_RECEIVING_SPEED);

	return (TowerCommSpeed) *(this->replyBuffer + 4);
}

VOID USBTowerController::SetReceivingSpeed(TowerCommSpeed speed)
{
	MakeRequest(
		TowerRequestType::SET_RECEIVING_SPEED,
		(BYTE)speed,
		0);
}

TowerTransmitterState USBTowerController::GetTransmitterState()
{
	MakeRequest(TowerRequestType::GET_TRANSMITTER_STATE);

	return (TowerTransmitterState) *(this->replyBuffer + 4);
}

TowerLEDColor USBTowerController::GetLEDColor(TowerLED led)
{
	BYTE loByte = (BYTE)led;
	BYTE hiByte = 0;
	MakeRequest(
		TowerRequestType::GET_LED,
		loByte,
		hiByte);

	return (TowerLEDColor) *(this->replyBuffer + 5);
}

VOID USBTowerController::SetLEDColor(TowerLED led, TowerLEDColor color)
{
	BYTE loByte = (BYTE)led;
	BYTE hiByte = (BYTE)color;
	MakeRequest(
		TowerRequestType::SET_LED,
		loByte,
		hiByte);
}

TowerCapabilitiesData USBTowerController::GetCapabilities(TowerCapabilityLink link)
{
	MakeRequest(TowerRequestType::GET_CAPABILITIES, (WORD)link);

	return *reinterpret_cast<TowerCapabilitiesData*>(this->replyBuffer + 4);
}

TowerVersionData USBTowerController::GetVersion()
{
	MakeRequest(TowerRequestType::GET_VERSION);

	return *reinterpret_cast<TowerVersionData*>(this->replyBuffer + 4);
}

VOID USBTowerController::GetCopyright(CHAR*& buffer, INT& length)
{
	MakeRequest(TowerRequestType::GET_COPYRIGHT);

	ReadStringFromReplyBuffer(buffer, length);
}

VOID USBTowerController::GetCredits(CHAR*& buffer, INT& length)
{
	MakeRequest(TowerRequestType::GET_CREDITS);

	ReadStringFromReplyBuffer(buffer, length);
}

VOID USBTowerController::ReadStringFromReplyBuffer(CHAR*& buffer, INT& length)
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

VOID USBTowerController::SetParameter(
	TowerParamType parameter,
	BYTE value)
{
	MakeRequest(
		TowerRequestType::SET_PARAMETER,
		(BYTE)parameter,
		value);
}

BYTE USBTowerController::GetParameter(TowerParamType parameter)
{
	MakeRequest(
		TowerRequestType::GET_PARAMETER,
		(BYTE)parameter,
		0);

	return *(this->replyBuffer + 3);
}

VOID USBTowerController::MakeRequest(TowerRequestType request)
{
	MakeRequest(request, 0);
}

VOID USBTowerController::MakeRequest(
	TowerRequestType request,
	BYTE loByte,
	BYTE hiByte)
{
	WORD value = ((hiByte << 8) | loByte);
	MakeRequest(request, value);
}

VOID USBTowerController::MakeRequest(
	TowerRequestType request,
	WORD value)
{
	// not used rn, will implement when used
	WORD index = 0;

	BOOL success = SendVendorRequest(
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

/* TODO: move lower level stuff out of here into its own little happy place */

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	WORD value,
	WORD index,
	USHORT bufferLength,
	BYTE* buffer,
	ULONG& lengthTransferred)
{
	WINUSB_SETUP_PACKET setupPacket;
	setupPacket.RequestType = 0xc0; // table 9-2 in https://fabiensanglard.net/usbcheat/usb1.1.pdf
	setupPacket.Request = request;
	setupPacket.Value = value;
	setupPacket.Index = index;
	setupPacket.Length = bufferLength;

	return WinUsb_ControlTransfer(
		*(this->handle),
		setupPacket,
		buffer,
		bufferLength,
		&lengthTransferred,
		NULL
	);
}
