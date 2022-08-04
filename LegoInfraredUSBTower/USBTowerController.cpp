#include "usbtowercontroller.h"
#include "TowerValues.h"
#include "LegoHeaders/LegoVendReq.h"

USBTowerController::USBTowerController(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;
}

TowerReqError USBTowerController::SetMode(TowerMode mode)
{
	return MakeRequest(
		TowerRequest::SET_PARAMETER,
		TowerParameter::MODE,
		(BYTE)mode);
}

TowerReqError USBTowerController::SetIndicatorLEDMode(TowerIndicatorLEDMode ledMode)
{
	return MakeRequest(
		TowerRequest::SET_PARAMETER,
		TowerParameter::INDICATOR_LED_MODE,
		(BYTE)ledMode);
}

TowerReqError USBTowerController::SetLED(TowerLED led, TowerLEDColor color)
{
	return MakeRequest(
		TowerRequest::SET_LED,
		(BYTE)led,
		(BYTE)color);
}


TowerReqError USBTowerController::MakeRequest(
	TowerRequest request,
	TowerParameter parameter,
	BYTE value)
{
	return MakeRequest(
		(BYTE)request,
		(BYTE)parameter,
		value,
		0);
}

TowerReqError USBTowerController::MakeRequest(
	TowerRequest request,
	BYTE parameter,
	BYTE value)
{
	return MakeRequest(
		(BYTE)request,
		parameter,
		value,
		0);
}

TowerReqError USBTowerController::MakeRequest(
	BYTE request,
	BYTE parameter,
	BYTE value)
{
	return MakeRequest(
		request,
		parameter,
		value,
		0);
}

TowerReqError USBTowerController::MakeRequest(
	BYTE request,
	BYTE parameter,
	BYTE value,
	WORD index)
{
	WORD replyLength = 8;
	BYTE replyBuffer[8];
	BOOL success = SendVendorRequest(
		request,
		parameter,
		value,
		index,
		replyLength,
		replyBuffer);

	if (!success)
	{
		return TowerReqError::BAD_PARAMETER;
	}

	TowerReqError error = (TowerReqError) *(replyBuffer + 2);
	return error;
}


/* TODO: move lower level stuff out of here into its own little happy place */

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	BYTE parameter,
	BYTE value,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	// the value takes the low byte and the parameter takes the high byte (little endian)
	return SendVendorRequest(
		request,
		((value << 8) | parameter),
		index,
		replyLength,
		replyBuffer
	);
}

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	BYTE parameter,
	BYTE value)
{
	// TODO: no
	BYTE replyBuffer[8];
	WORD replyLength = 8;
	return SendVendorRequest(
		request,
		parameter,
		value,
		0,
		replyLength,
		replyBuffer
	);
}

//BOOL USBTowerController::SendVendorRequest(
//	BYTE request,
//	WORD replyLength,
//	BYTE* replyBuffer)
//{
//	BYTE replyBuffer[8];
//	WORD replyLength = 8;
//	return USBTowerController::SendVendorRequest(
//		request,
//		0,
//		0,
//		0,
//		replyLength,
//		replyBuffer
//	);
//}

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	WORD fullValue,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	WINUSB_SETUP_PACKET setupPacket;
	setupPacket.RequestType = 0xc0; // table 9-2 in https://fabiensanglard.net/usbcheat/usb1.1.pdf
	setupPacket.Request = request;
	setupPacket.Value = fullValue;
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
