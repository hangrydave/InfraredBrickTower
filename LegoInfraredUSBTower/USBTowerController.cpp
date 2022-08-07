#include "usbtowercontroller.h"
#include "TowerValues.h"
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

TowerRequestError USBTowerController::GetLastRequestError()
{
	return lastRequestError;
}

TowerLEDColor USBTowerController::GetLEDColor(TowerLED led)
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_LED,
		(BYTE)led,
		0,
		0);

	return (TowerLEDColor) *(this->replyBuffer + 5);
}

VOID USBTowerController::SetLEDColor(TowerLED led, TowerLEDColor color)
{
	MakeRequest(
		(BYTE)TowerRequestType::SET_LED,
		(BYTE)led,
		(BYTE)color,
		0);
}

TowerCapabilitiesData USBTowerController::GetCapabilities(TowerCapabilityLink link)
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_CAPABILITIES,
		(WORD)link,
		0,
		0);

	return *reinterpret_cast<TowerCapabilitiesData*>(this->replyBuffer + 4);
}

TowerVersionData USBTowerController::GetVersion()
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_VERSION,
		0,
		0);

	return *reinterpret_cast<TowerVersionData*>(this->replyBuffer + 4);
}

VOID USBTowerController::GetCopyright(CHAR*& buffer, INT& length)
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_COPYRIGHT,
		0,
		0);

	ReadStringFromReplyBuffer(buffer, length);
}

VOID USBTowerController::GetCredits(CHAR*& buffer, INT& length)
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_CREDITS,
		0,
		0);

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
		(BYTE)TowerRequestType::SET_PARAMETER,
		(BYTE)parameter,
		value,
		0);
}

BYTE USBTowerController::GetParameter(TowerParamType parameter)
{
	MakeRequest(
		(BYTE)TowerRequestType::GET_PARAMETER,
		(BYTE)parameter,
		0);

	return *(this->replyBuffer + 3);
}

//WORD USBTowerController::BuildValue(BYTE loByte, BYTE hiByte)
//{
//	return ((hiByte << 8) | loByte);
//}

VOID USBTowerController::MakeRequest(
	BYTE request,
	BYTE loByte,
	BYTE hiByte,
	WORD index)
{
	MakeRequest(
		request,
		((hiByte << 8) | loByte),
		index);
}

VOID USBTowerController::MakeRequest(
	BYTE request,
	WORD value,
	WORD index)
{
	/*if (expectedReplyLength <= 0)
	{
		expectedReplyLength = 8;
	}

	if (this->replyBufferSize < expectedReplyLength)
	{
		delete this->replyBuffer;
		this->replyBuffer = new BYTE[expectedReplyLength];
		this->replyBufferSize = expectedReplyLength;
	}
	else if (this->replyBufferSize == 0)
	{
		delete this->replyBuffer;
		this->replyBuffer = new BYTE[8];
		this->replyBufferSize = 8;
	}*/

	BOOL success = SendVendorRequest(
		request,
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
