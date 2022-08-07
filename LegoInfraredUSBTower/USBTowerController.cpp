#include "usbtowercontroller.h"
#include "TowerValues.h"
#include "LegoHeaders/LegoVendReq.h"
#include <string>

USBTowerController::USBTowerController(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;
	this->lastRequestError = TowerRequestError::SUCCESS;
}

TowerRequestError USBTowerController::GetLastRequestError()
{
	return lastRequestError;
}

TowerLEDColor USBTowerController::GetLEDColor(TowerLED led)
{
	BYTE replyBuffer[6];
	WORD replyLength = 6;
	MakeRequest(
		(BYTE)TowerRequestType::GET_LED,
		(BYTE)led,
		0,
		0,
		replyLength,
		replyBuffer);

	return (TowerLEDColor) *(replyBuffer + 5);
}

VOID USBTowerController::SetLEDColor(TowerLED led, TowerLEDColor color)
{
	MakeRequest(
		(BYTE)TowerRequestType::SET_LED,
		(BYTE)led,
		(BYTE)color);
}

TowerCapabilitiesData USBTowerController::GetCapabilities(TowerCapabilityLink link)
{
	BYTE replyBuffer[18];
	WORD replyLength = 18;
	MakeRequest(
		(BYTE)TowerRequestType::GET_CAPABILITIES,
		(WORD)link,
		0,
		replyLength,
		replyBuffer);

	return *reinterpret_cast<TowerCapabilitiesData*>(replyBuffer + 4);
}

TowerVersionData USBTowerController::GetVersion()
{
	BYTE replyBuffer[8];
	WORD replyLength = 8;
	MakeRequest(
		(BYTE)TowerRequestType::GET_VERSION,
		0,
		0,
		replyLength,
		replyBuffer);

	return *reinterpret_cast<TowerVersionData*>(replyBuffer + 4);
}

VOID USBTowerController::GetCopyright(CHAR*& buffer, INT& length)
{
	BYTE replyBuffer[127];
	WORD replyLength = 127;

	MakeRequest(
		(BYTE)TowerRequestType::GET_COPYRIGHT,
		0,
		0,
		replyLength,
		replyBuffer);

	length = 0;
	buffer = new CHAR[replyLength / 2];

	for (int i = 4; i < replyLength; i++)
	{
		char c = replyBuffer[i];

		if (c == '\0' && i < replyLength - 1)
		{
			char prev = replyBuffer[i - 1];
			char next = replyBuffer[i + 1];

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

VOID USBTowerController::GetCredits(CHAR*& buffer, INT& length)
{
	BYTE replyBuffer[917];
	WORD replyLength = 917;

	MakeRequest(
		(BYTE)TowerRequestType::GET_CREDITS,
		0,
		0,
		replyLength,
		replyBuffer);

	length = 0;
	buffer = new CHAR[replyLength / 2];

	for (int i = 4; i < replyLength; i++)
	{
		char c = replyBuffer[i];

		if (c == '\0' && i < replyLength - 1)
		{
			char prev = replyBuffer[i - 1];
			char next = replyBuffer[i + 1];

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

//VOID USBTowerController::ReadString(CHAR*& buffer, INT& length)
//{
//
//	SendVendorRequest
//}

VOID USBTowerController::SetParameter(
	TowerParamType parameter,
	BYTE value)
{
	MakeRequest(
		(BYTE)TowerRequestType::SET_PARAMETER,
		(BYTE)parameter,
		value);
}

BYTE USBTowerController::GetParameter(TowerParamType parameter)
{
	BYTE replyBuffer[4];
	WORD replyLength = 4;
	MakeRequest(
		(BYTE)TowerRequestType::GET_PARAMETER,
		(BYTE)parameter,
		0,
		0,
		replyLength,
		replyBuffer);

	return *(replyBuffer + 3);
}

//VOID USBTowerController::MakeRequest(
//	BYTE request,
//	BYTE loByte,
//	BYTE hiByte)
//{
//	BYTE replyBuffer[8];
//	WORD replyLength = 8;
//	MakeRequest(
//		(BYTE)request,
//		loByte,
//		hiByte,
//		0,
//		replyLength,
//		replyBuffer);
//}

VOID USBTowerController::MakeRequest(
	BYTE request,
	WORD value,
	WORD index,
	ULONG expectedReplyLength)
{
	BOOL success = SendVendorRequest(
		request,
		value,
		index,
		expectedReplyLength,
		this->lastReplyBuffer,
		this->lastReplyLength);

	if (!success)
	{
		this->lastRequestError = TowerRequestError::BAD_PARAMETER;
	}
	else
	{
		BYTE errorByte = *(this->lastReplyBuffer + 2);
		this->lastRequestError = (TowerRequestError)errorByte;
	}
}

VOID USBTowerController::MakeRequest(
	BYTE request,
	BYTE loByte,
	BYTE hiByte,
	WORD index,
	ULONG expectedReplyLength)
{
	MakeRequest(
		request,
		((hiByte << 8) | loByte),
		index);
}

/* TODO: move lower level stuff out of here into its own little happy place */

//BOOL USBTowerController::SendVendorRequest(
//	BYTE request,
//	BYTE loByte,
//	BYTE hiByte,
//	WORD index,
//	WORD replyLength,
//	BYTE* replyBuffer,
//	ULONG& lengthTransferred)
//{
//	// the hiByte takes the low byte and the loByte takes the high byte (little endian)
//	return SendVendorRequest(
//		request,
//		((hiByte << 8) | loByte),
//		index,
//		replyLength,
//		replyBuffer,
//		lengthTransferred
//	);
//}

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	WORD value,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer,
	ULONG& lengthTransferred)
{
	WINUSB_SETUP_PACKET setupPacket;
	setupPacket.RequestType = 0xc0; // table 9-2 in https://fabiensanglard.net/usbcheat/usb1.1.pdf
	setupPacket.Request = request;
	setupPacket.Value = value;
	setupPacket.Index = index;
	setupPacket.Length = replyLength;

	return WinUsb_ControlTransfer(
		*(this->handle),
		setupPacket,
		replyBuffer,
		replyLength,
		&lengthTransferred,
		NULL
	);
}
