#include "usbtowercontroller.h"
#include "TowerValues.h"
#include "LegoHeaders/LegoVendReq.h"

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

VOID USBTowerController::MakeRequest(
	BYTE request,
	BYTE loByte,
	BYTE hiByte)
{
	BYTE replyBuffer[8];
	WORD replyLength = 8;
	MakeRequest(
		(BYTE)request,
		loByte,
		hiByte,
		0,
		replyLength,
		replyBuffer);
}

VOID USBTowerController::MakeRequest(
	BYTE request,
	BYTE loByte,
	BYTE hiByte,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	BOOL success = SendVendorRequest(
		request,
		loByte,
		hiByte,
		index,
		replyLength,
		replyBuffer);

	if (!success)
	{
		this->lastRequestError = TowerRequestError::BAD_PARAMETER;
	}
	else
	{
		BYTE errorByte = *(replyBuffer + 2);
		this->lastRequestError = (TowerRequestError)errorByte;
	}
}

/* TODO: move lower level stuff out of here into its own little happy place */

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	BYTE loByte,
	BYTE hiByte,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	// the hiByte takes the low byte and the loByte takes the high byte (little endian)
	return SendVendorRequest(
		request,
		((hiByte << 8) | loByte),
		index,
		replyLength,
		replyBuffer
	);
}

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	WORD value,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	WINUSB_SETUP_PACKET setupPacket;
	setupPacket.RequestType = 0xc0; // table 9-2 in https://fabiensanglard.net/usbcheat/usb1.1.pdf
	setupPacket.Request = request;
	setupPacket.Value = value;
	setupPacket.Index = index;
	setupPacket.Length = replyLength;

	ULONG lengthTransferred;
	return WinUsb_ControlTransfer(
		*(this->handle),
		setupPacket,
		replyBuffer,
		replyLength,
		&lengthTransferred,
		NULL
	);
}
