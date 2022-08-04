#pragma once

#ifndef USBTOWERCONTROLLER_H
#define USBTOWERCONTROLLER_H

#include <Windows.h>
#include <winusb.h>
#include "TowerValues.h"

enum class TowerRequest : BYTE
{
	GET_PARAMETER = 0x01,
	SET_PARAMETER = 0x02,
	FLUSH = 0x03,
	RESET = 0x04,
	GET_STATISTICS = 0x05,
	GET_POWER = 0x06,
	GET_LED = 0x08,
	SET_LED = 0x09,
	RESET_STATISTICS = 0x10,
	GET_IRC_PARAMETER = 0x11,
	SET_IRC_PARAMETER = 0x12,
	GET_TRANSMISSION_SPEED = 0xEE,
	SET_TRANSMISSION_SPEED = 0xEF,
	GET_RECEIVING_SPEED = 0xF0,
	SET_RECEIVING_SPEED = 0xF1,
	GET_TRANSMITTER_STATE = 0xF2,
	GET_TRANSMISSION_CARRIER_FREQUENCY = 0xF3,
	SET_TRANSMISSION_CARRIER_FREQUENCY = 0xF4,
	GET_TRANSMISSION_CARRIER_DUTY_CYCLE = 0xF5,
	SET_TRANSMISSION_CARRIER_DUTY_CYCLE = 0xF6,
	GET_CAPABILITIES = 0xFC,
	GET_VERSION = 0xFD,
	GET_COPYRIGHT = 0xFE,
	GET_CREDITS = 0xFF
};

enum class TowerReqError : BYTE
{
	SUCCESS = 0x00,
	BAD_PARAMETER = 0x01,
	BUSY = 0x02,
	NOT_ENOUGH_POWER = 0x03,
	WRONG_MODE = 0x04,
	INTERNAL_ERROR = 0xFE,
	BAD_REQUEST = 0xFF
};

enum class TowerParameter : BYTE
{
	MODE = 0x01,
	RANGE = 0x02,
	ERROR_DETECTION = 0x03,
	ERROR_STATUS = 0x04,
	ENDIAN = 0x97,
	INDICATOR_LED_MODE = 0x98,
	ERROR_SIGNAL = 0x99
};

enum class TowerMode : BYTE
{
	VLL = 0x01,
	IR = 0x02,
	IRC = 0x04,
	RADIO = 0x08
};

enum class TowerRange : BYTE
{
	SHORT = 0x01,
	MEDIUM = 0x02,
	LONG = 0x03
};

enum class TowerErrorDetection : BYTE
{
	ON = 0x01,
	OFF = 0x02
};

enum class TowerErrorStatus : BYTE
{
	OK = 0,
	// Function/parameter error
	NO_SUPPORT_FOR_FUNCTION = 1,
	INVALID_VENDOR_REQUEST = 10,
	// USB errors
	UNSPECIFIED_USB = 100,
	WRONG_USB_FUNCTION = 101,
	NO_OR_INVALID_USB_DATA = 102,
	WRONG_USB_ENDPOINT = 103,
	INVALID_PACKET_SIZE = 104,
	ERROR_SENDING_ON_ENDPOINT_0 = 105,
	SENT_TOO_MUCH_ON_ENDPOINT_0 = 106,
	SENT_TOO_MUCH_ON_ENDPOINT_1 = 107,
	VENDOR_ANSWER_BUFFER_FULL = 108,
	ERROR_SENDING_ON_ENDPOINT_1 = 109,
	// UART errors
	UART_PROBLEMS = 200,
	INVALID_UART_MODE = 201,
	INVALID_TX_STATE = 210,
	CANT_SEND_LONG_RANGE_WHEN_LOW_POWER = 211,
	// Tower errors
	TOWER_OUT_OF_ORDER = 240,
	UNDEFINED = 255
};

enum class TowerEndian : BYTE
{
	LITTLE = 0x01,
	BIG = 0x02
};

enum class TowerIndicatorLEDMode : BYTE
{
	FIRMWARE_CONTROLLED = 0x01,
	HOST_SOFTWARE_CONTROLLED = 0x02
};

enum class TowerErrorSignal : BYTE
{
	LED_ON = 0x01,
	LED_OFF = 0x02
};

enum class TowerBuffer : BYTE
{
	TRANSMISSION_BUFFER = 0x01,
	RECEIVER_BUFFER = 0x02,
	ALL_BUFFERS = 0x03
};

enum class TowerPower : BYTE
{
	LOW = 0x01,
	HIGH = 0x02
};

enum class TowerLED : BYTE
{
	ID = 0x01,
	VLL = 0x02
};

enum class TowerLEDColor : BYTE
{
	OFF = 0x00,
	GREEN = 0x01,
	YELLOW = 0x02,
	ORANGE = 0x04,
	RED = 0x08,
	DEFAULT = 0xFF
};

enum class TowerIRCParameter : BYTE
{
	PACKET_SIZE = 0x01,
	TRANSMISSION_DELAY = 0x02
};

enum class TowerCommSpeed : BYTE
{
	COMM_BAUD_1200 = 0x0004,
	COMM_BAUD_2400 = 0x0008,
	COMM_BAUD_4800 = 0x0010,
	COMM_BAUD_9600 = 0x0020,
	COMM_BAUD_19200 = 0x0040
};

enum class TowerTransmitterState : BYTE
{
	READY = 0x01,
	BUSY = 0x02
};

enum class TowerCapability : BYTE
{
	VLL = 0x01,
	IR = 0x02,
	IRC = 0x04,
	RADIO = 0x08
};

enum class TowerCapabilityCommDirection : BYTE
{
	TRANSMIT = 0x01,
	RECEIVE = 0x02,
	BOTH = 0x03
};

enum class TowerCapabilityCommRange : BYTE
{
	SHORT = 0x01,
	MEDIUM = 0x02,
	LONG = 0x04,
	ALL = 0x07
};

enum class TowerCapabilityCommSpeed : BYTE
{
	COMM_BAUD_1200 = 0x0004,
	COMM_BAUD_2400 = 0x0008,
	COMM_BAUD_4800 = 0x0010,
	COMM_BAUD_9600 = 0x0020,
	COMM_BAUD_19200 = 0x0040
};

class USBTowerController
{
public:
	USBTowerController(const WINUSB_INTERFACE_HANDLE* handle);

	TowerReqError GetMode(TowerMode& mode);
	TowerReqError GetRange(TowerRange& range);
	TowerReqError GetErrorDetection(TowerErrorDetection& errorDetection);
	TowerReqError GetErrorStatus(TowerErrorStatus& errorStatus);
	TowerReqError GetEndian(TowerEndian& endian);
	TowerReqError GetIndicatorLEDMode(TowerIndicatorLEDMode& ledMode);
	TowerReqError GetErrorSignal(TowerErrorSignal& errorSignal);

	TowerReqError SetMode(TowerMode mode);
	TowerReqError SetRange(TowerRange range);
	TowerReqError SetErrorDetection(TowerErrorDetection errorDetection);
	TowerReqError SetErrorStatus(TowerErrorStatus errorStatus);
	TowerReqError SetEndian(TowerEndian endian);
	TowerReqError SetIndicatorLEDMode(TowerIndicatorLEDMode ledMode);
	TowerReqError SetErrorSignal(TowerErrorSignal errorSignal);

	TowerReqError Flush(TowerBuffer buffer);
	TowerReqError Reset();

	TowerReqError GetPower();

	TowerReqError GetLED();
	TowerReqError SetLED(TowerLED led, TowerLEDColor color);

	TowerReqError GetStatistics();
	TowerReqError ResetStatistics();

	TowerReqError GetIRCParameter();
	TowerReqError SetIRCParameter();

	TowerReqError GetTransmissionSpeed();
	TowerReqError SetTransmissionSpeed();

	TowerReqError GetReceivingSpeed();
	TowerReqError SetReceivingSpeed();

	TowerReqError GetTransmitterState();

	TowerReqError GetTransmissionCarrierFrequency();
	TowerReqError SetTransmissionCarrierFrequency();
	TowerReqError GetTransmissionCarrierDutyCycle();
	TowerReqError SetTransmissionCarrierDutyCycle();

	TowerReqError GetCapabilities();
	TowerReqError GetVersion();
	TowerReqError GetCopyright();
	TowerReqError GetCredits();
private:
	const WINUSB_INTERFACE_HANDLE* handle;

	TowerReqError MakeRequest(
		TowerRequest request,
		TowerParameter parameter,
		BYTE value);

	TowerReqError MakeRequest(
		TowerRequest request,
		BYTE parameter,
		BYTE value);

	TowerReqError MakeRequest(
		BYTE request,
		BYTE parameter,
		BYTE value);

	TowerReqError MakeRequest(
		BYTE request,
		BYTE parameter,
		BYTE value,
		WORD index);

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value,
		WORD index,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value);

	BOOL SendVendorRequest(
		BYTE request,
		WORD fullValue,
		WORD index,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		WORD fullValue,
		WORD replyLength,
		BYTE* replyBuffer);

	/*BOOL SendVendorRequest(
		BYTE request,
		WORD replyLength,
		BYTE* replyBuffer);*/
};

#endif USBTOWERCONTROLLER_H
