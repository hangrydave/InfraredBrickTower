#pragma once

#include <stdio.h>
#include <Windows.h>
#include "HostTowerCommInterface.h"

namespace Tower
{
	enum class RequestType : BYTE
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

	enum class RequestError : BYTE
	{
		SUCCESS = 0x00,
		BAD_PARAMETER = 0x01,
		BUSY = 0x02,
		NOT_ENOUGH_POWER = 0x03,
		WRONG_MODE = 0x04,
		INTERNAL_ERROR = 0xFE,
		BAD_REQUEST = 0xFF
	};

	enum class ParamType : BYTE
	{
		MODE = 0x01,
		RANGE = 0x02,
		ERROR_DETECTION = 0x03,
		ERROR_STATUS = 0x04,
		ENDIAN = 0x97,
		INDICATOR_LED_MODE = 0x98,
		ERROR_SIGNAL = 0x99
	};

	enum class CommMode : BYTE
	{
		VLL = 0x01,
		IR = 0x02,
		IRC = 0x04,
		RADIO = 0x08
	};

	enum class CommRange : BYTE
	{
		SHORT = 0x01,
		MEDIUM = 0x02,
		LONG = 0x03
	};

	enum class ErrorDetectionMode : BYTE
	{
		ON = 0x01,
		OFF = 0x02
	};

	enum class ErrorStatus : BYTE
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

	enum class Endian : BYTE
	{
		LITTLE = 0x01,
		BIG = 0x02
	};

	enum class IndicatorLEDMode : BYTE
	{
		FIRMWARE_CONTROLLED = 0x01,
		HOST_SOFTWARE_CONTROLLED = 0x02
	};

	enum class ErrorSignalMode : BYTE
	{
		LED_ON = 0x01,
		LED_OFF = 0x02
	};

	enum class CommBuffer : BYTE
	{
		TRANSMISSION_BUFFER = 0x01,
		RECEIVER_BUFFER = 0x02,
		ALL_BUFFERS = 0x03
	};

	enum class Power : BYTE
	{
		LOW = 0x01,
		HIGH = 0x02
	};

	enum class LED : BYTE
	{
		ID = 0x01,
		VLL = 0x02
	};

	enum class LEDColor : BYTE
	{
		OFF = 0x00,
		GREEN = 0x01,
		YELLOW = 0x02,
		ORANGE = 0x04,
		RED = 0x08,
		DEFAULT = 0xFF
	};

	enum class IRCParam : BYTE
	{
		PACKET_SIZE = 0x01,
		TRANSMISSION_DELAY = 0x02
	};

	enum class CommSpeed : WORD
	{
		COMM_BAUD_1200 = 0x0004,
		COMM_BAUD_2400 = 0x0008,
		COMM_BAUD_4800 = 0x0010,
		COMM_BAUD_9600 = 0x0020,
		COMM_BAUD_19200 = 0x0040
	};

	enum class TransmitterState : BYTE
	{
		READY = 0x01,
		BUSY = 0x02
	};

	enum class CapabilityLink : BYTE
	{
		VLL = 0x01,
		IR = 0x02,
		IRC = 0x04
		//RADIO = 0x08		unsupported
	};

	enum class CapabilityCommDirection : BYTE
	{
		TRANSMIT = 0x01,
		RECEIVE = 0x02,
		BOTH = 0x03
	};

	enum class CapabilityCommRange : BYTE
	{
		SHORT = 0x01,
		MEDIUM = 0x02,
		LONG = 0x04,
		ALL = 0x07
	};

	enum class CapabilityCommSpeed : WORD
	{
		COMM_BAUD_1200 = 0x0004,
		COMM_BAUD_2400 = 0x0008,
		COMM_BAUD_4800 = 0x0010,
		COMM_BAUD_9600 = 0x0020,
		COMM_BAUD_19200 = 0x0040
	};

#pragma pack(push, 1)
	struct CapabilitiesData
	{
		CapabilityCommDirection direction;
		CapabilityCommRange range;
		CapabilityCommSpeed transmitRate;
		CapabilityCommSpeed receiveRate;
		BYTE minCarrierFrequency; // what is this
		BYTE maxCarrierFrequency; // what is this
		WORD minDutyCycle; // what is this
		WORD maxDutyCycle; // what is this
		BYTE UARTTransmitBufferSize;
		BYTE UARTReceiveBufferSize;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct VersionData
	{
		BYTE majorVersion;
		BYTE minorVersion;
		WORD buildNumber;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct StatisticsData
	{
		WORD receivedBytesCount;
		WORD overrunErrorCount;
		WORD noiseCount;
		WORD framingErrorCount;
	};
#pragma pack(pop)

#define REQUEST_REPLY_BUFFER_LENGTH 1000
		
	struct RequestData
	{
		HostTowerCommInterface* commInterface;

		RequestError lastRequestError = RequestError::SUCCESS;
		ULONG lastReplyLength = 0;
		BYTE replyBuffer[REQUEST_REPLY_BUFFER_LENGTH];
		ULONG stringLength = 0;
		WCHAR stringBuffer[REQUEST_REPLY_BUFFER_LENGTH];

		RequestData(HostTowerCommInterface* commInterface)
		{
			this->commInterface = commInterface;
			*replyBuffer = {};
			*stringBuffer = {};
		}

		~RequestData(){ }
	};

	BOOL WriteData(PUCHAR buffer, ULONG bufferLength, ULONG& lengthWritten, RequestData* data);
	BOOL ReadData(PUCHAR buffer, ULONG bufferLength, ULONG& lengthRead, RequestData* data);

	VOID Flush(CommBuffer buffer, RequestData* data);
	VOID Reset(RequestData* data);

	Power GetPower(RequestData* data);

	LEDColor GetLEDColor(LED led, RequestData* data);
	VOID SetLEDColor(LED led, LEDColor color, RequestData* data);

	StatisticsData GetStatistics(RequestData* data);
	VOID ResetStatistics(RequestData* data);

	/*TowerIRCParam GetIRCParameter();
	VOID SetIRCParameter(TowerIRCParam param);*/

	CommSpeed GetTransmissionSpeed(RequestData* data);
	VOID SetTransmissionSpeed(CommSpeed speed, RequestData* data);

	CommSpeed GetReceivingSpeed(RequestData* data);
	VOID SetReceivingSpeed(CommSpeed speed, RequestData* data);

	TransmitterState GetTransmitterState(RequestData* data);

	/*BYTE GetTransmissionCarrierFrequency();
	VOID SetTransmissionCarrierFrequency();
	BYTE GetTransmissionCarrierDutyCycle();
	VOID SetTransmissionCarrierDutyCycle();*/

	CapabilitiesData GetCapabilities(CapabilityLink link, RequestData* data);
	VersionData GetVersion(RequestData* data);
	VOID GetCopyright(RequestData* data);
	VOID GetCredits(RequestData* data);

	VOID ReadStringFromReplyBuffer(RequestData* data);

	VOID SetParameter(
		ParamType parameter,
		BYTE value,
		RequestData* data);
	BYTE GetParameter(ParamType parameter, RequestData* data);

	VOID MakeRequest(RequestType request, RequestData* data);
	VOID MakeRequest(
		RequestType request,
		WORD value,
		RequestData* data);
	VOID MakeRequest(
		RequestType request,
		BYTE loByte,
		BYTE hiByte,
		RequestData* data);

	/*
		well...
		I'm using a macro to generate the getters and setters for the GET_PARAMETER and SET_PARAMETER requests.

		Q: why?
		A: I didn't want to write a bunch of cookie cutter getters and setters for those.
		Q: was it a good decision?
		A: beats me if the broader C++ Community:TM: would approve! My reasoning is this:
		   - realistically, are many people who aren't me going to use this? probably not, so does it matter? probably not
		   - i would rather have a little bit of weird macro stuff than a bunch of space filled by near-identical methods
		   - i wanted to learn macros

		anyway, this might be bad, idk and tbqh idc
	*/
#define GenerateParameterSetterAndGetter(paramType, outputType) \
inline outputType Get##outputType##(RequestData* data) \
{ \
	return (##outputType) GetParameter(##paramType##, data); \
} \
inline VOID Set##outputType##(##outputType newValue, RequestData* data) \
{ \
	SetParameter(##paramType##, (BYTE)newValue, data); \
}

	GenerateParameterSetterAndGetter(ParamType::MODE, CommMode)
	GenerateParameterSetterAndGetter(ParamType::RANGE, CommRange)
	GenerateParameterSetterAndGetter(ParamType::ERROR_DETECTION, ErrorDetectionMode)
	GenerateParameterSetterAndGetter(ParamType::ERROR_STATUS, ErrorStatus)
	GenerateParameterSetterAndGetter(ParamType::ENDIAN, Endian)
	GenerateParameterSetterAndGetter(ParamType::INDICATOR_LED_MODE, IndicatorLEDMode)
	GenerateParameterSetterAndGetter(ParamType::ERROR_SIGNAL, ErrorSignalMode)
}
