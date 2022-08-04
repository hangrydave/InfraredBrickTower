#include "usbtowercontroller.h"

USBTowerController::USBTowerController(const WINUSB_INTERFACE_HANDLE* handle)
{
	this->handle = handle;
}

BOOL USBTowerController::BlinkLights()
{
	USBTowerController::SendVendorRequest(LTW_REQ_SET_PARM, LTW_PARM_ID_LED_MODE, LTW_ID_LED_SW_CTRL);
	USBTowerController::SendVendorRequest(LTW_REQ_SET_LED, LTW_LED_VLL, LTW_LED_COLOR_ON);
	USBTowerController::SendVendorRequest(LTW_REQ_SET_LED, LTW_LED_ID, LTW_LED_COLOR_ON);
	USBTowerController::SendVendorRequest(LTW_REQ_SET_LED, LTW_LED_VLL, LTW_LED_COLOR_OFF);
	USBTowerController::SendVendorRequest(LTW_REQ_SET_LED, LTW_LED_ID, LTW_LED_COLOR_OFF);
	return TRUE;
}

BOOL USBTowerController::GetVersion(LTW_REQ_GET_VERSION_REPLY& reply)
{
	BYTE replyBuffer[sizeof(LTW_REQ_GET_VERSION_REPLY)];
	BOOL success = USBTowerController::SendVendorRequest(
		LTW_REQ_GET_VERSION,
		0,
		0,
		0,
		sizeof(LTW_REQ_GET_VERSION_REPLY),
		replyBuffer
	);

	if (!success)
		return FALSE;

	reply = *reinterpret_cast<LTW_REQ_GET_VERSION_REPLY*>(replyBuffer);
	return TRUE;
}

BOOL USBTowerController::SendVendorRequest(
	BYTE request,
	BYTE parameter,
	BYTE value,
	WORD index,
	WORD replyLength,
	BYTE* replyBuffer)
{
	// the value takes the low byte and the parameter takes the high byte (little endian)
	return USBTowerController::SendVendorRequest(
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
	BYTE replyBuffer[8];
	WORD replyLength = 8;
	return USBTowerController::SendVendorRequest(
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
	WinUsb_ControlTransfer(
		*(this->handle),
		setupPacket,
		replyBuffer,
		replyLength,
		&lengthTransferred,
		NULL
	);

	return TRUE;
}
