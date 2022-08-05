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

TowerMode USBTowerController::GetMode()
{
	return (TowerMode) GetParameter(TowerParamType::MODE);
}

VOID USBTowerController::SetMode(TowerMode mode)
{
	SetParameter(TowerParamType::MODE, (BYTE)mode);
}

VOID USBTowerController::SetIndicatorLEDMode(TowerIndicatorLEDMode ledMode)
{
	SetParameter(TowerParamType::INDICATOR_LED_MODE, (BYTE)ledMode);
}

VOID USBTowerController::SetLED(TowerLED led, TowerLEDColor color)
{
	lastRequestError = MakeRequest(
		(BYTE)TowerRequestType::SET_LED,
		(BYTE)led,
		(BYTE)color);
}

//TowerReqError USBTowerController::MakeRequest(
//	TowerRequest request,
//	TowerParameter parameter,
//	BYTE& outValue)
//{
//
//	return MakeRequest(
//		request,
//		parameter,
//		replyLength,
//		replyBuffer);
//}

//TowerReqError USBTowerController::MakeRequest(
//	TowerRequest request,
//	TowerParameter parameter,
//	WORD replyLength,
//	BYTE* replyBuffer)
//{
//	return MakeRequest(
//		request,
//		parameter,
//		replyLength,
//		replyBuffer);
//}

//TowerReqError USBTowerController::MakeRequest(
//	TowerRequest request,
//	TowerParameter parameter,
//	BYTE value)
//{
//	return MakeRequest(
//		(BYTE)request,
//		(BYTE)parameter,
//		value,
//		0);
//}

//TowerReqError USBTowerController::MakeRequest(
//	TowerRequest request,
//	BYTE parameter,
//	BYTE value)
//{
//	return MakeRequest(
//		(BYTE)request,
//		parameter,
//		value,
//		0);
//}

//TowerReqError USBTowerController::MakeRequest(
//	BYTE request,
//	BYTE parameter,
//	BYTE value)
//{
//	return MakeRequest(
//		request,
//		parameter,
//		value,
//		0);
//}

//TowerReqError USBTowerController::MakeRequest(
//	BYTE request,
//	BYTE parameter,
//	BYTE value,
//	WORD index)
//{
//	BYTE replyBuffer[8];
//	WORD replyLength = 8;
//	return MakeRequest(
//		request,
//		parameter,
//		value,
//		index,
//		replyLength,
//		replyBuffer);
//}

VOID USBTowerController::SetParameter(
	TowerParamType parameter,
	BYTE value)
{
	lastRequestError = MakeRequest(
		(BYTE)TowerRequestType::SET_PARAMETER,
		(BYTE)parameter,
		value);
}

BYTE USBTowerController::GetParameter(TowerParamType parameter)
{
	BYTE replyBuffer[4];
	WORD replyLength = 4;
	lastRequestError = MakeRequest(
		(BYTE)TowerRequestType::GET_PARAMETER,
		(BYTE)parameter,
		0,
		0,
		replyLength,
		replyBuffer);

	return *(replyBuffer + 3); 
}

TowerRequestError USBTowerController::MakeRequest(
	BYTE request,
	BYTE parameter,
	BYTE value)
{
	BYTE replyBuffer[8];
	WORD replyLength = 8;
	TowerRequestError error = MakeRequest(
		(BYTE)request,
		(BYTE)parameter,
		value,
		0,
		replyLength,
		replyBuffer);

	return error;
}

TowerRequestError USBTowerController::MakeRequest(
	BYTE request,
	BYTE parameter,
	BYTE value,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	BOOL success = SendVendorRequest(
		request,
		parameter,
		value,
		index,
		replyLength,
		replyBuffer);

	if (!success)
	{
		return TowerRequestError::BAD_PARAMETER;
	}

	BYTE errorByte = *(replyBuffer + 2);
	return (TowerRequestError)errorByte;
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

//BOOL USBTowerController::SendVendorRequest(
//	BYTE request,
//	BYTE parameter,
//	BYTE value)
//{
//	// TODO: no
//	BYTE replyBuffer[8];
//	WORD replyLength = 8;
//	return SendVendorRequest(
//		request,
//		parameter,
//		value,
//		0,
//		replyLength,
//		replyBuffer
//	);
//}

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
