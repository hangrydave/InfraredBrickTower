#pragma once


/**************** (c) 1999            The LEGO Group **********************

PROJECT			: LEGO USB Tower

MODULE			: VendReq.h

AUTHOR			: LEGO Electronics R&D, AS & PPh

DESCRIPTION		: USB Vendor request structures

REMARK:	 1. Before you add this header file to your program you have to define
			the following datatypes:
			Byte = unsigned char (8 bits)
			Word = unsigned word (16 bits)

		 2. The Reply format has to be a even number of bytes. Extra bytes called "spare"
			are used to obtain a even number of bytes.

******************************************************************************/

#ifndef LTW_VENDREQ_H
#define LTW_VENDREQ_H

#define Word unsigned short
#define Byte unsigned char

////////////////////////////////////////////////////////////////////////////////////////////////////
//	Definition of Word/Byte for Win32
//	
//	Feel free to add definition for other platform -- as long as a word is 16 bits and a byte is
//	8 bits.
//	
////////////////////////////////////////////////////////////////////////////////////////////////////

//#ifndef __LTW_VENDREQ_NOTYPEDEFS__		// define this if you have already defined Word/Byte somewhere else
//
//	#ifdef _WIN32
//
//		typedef unsigned short Word;
//		typedef unsigned char  Byte;
//		typedef unsigned long  LWord;
//
//	#endif
//
//#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS TYPES
////////////////////////////////////////////////////////////////////////////////////////////////////

// USB Vendor requests 
//
#define LTW_REQ_GET_PARM     		0x01 // Get parameter for standard IR mode	
#define	LTW_REQ_SET_PARM			0x02 // Set parameter for standard IR mode
#define	LTW_REQ_FLUSH				0x03 // Flush UART buffers
#define LTW_REQ_RESET				0x04 // Performing a user reset
#define LTW_REQ_GET_STAT			0x05 // Get statistics on IR receiver
#define LTW_REQ_GET_POWER			0x06 // Get current powermode
#define LTW_REQ_GET_LED				0x08 // Get led state and color
#define LTW_REQ_SET_LED				0x09 // Switch on/off led + color of led
#define LTW_REQ_RESET_STAT			0x10 // Reset statictics on IR receiver
#define LTW_REQ_GET_PARM_IRC   		0x11 // Get parameter for IRC mode	
#define	LTW_REQ_SET_PARM_IRC		0x12 // Set parameter for IRC mode
#define LTW_REQ_GET_TX_SPEED		0xEE // Get IR transmission speed
#define LTW_REQ_SET_TX_SPEED		0xEF // Set IR transmission speed
#define LTW_REQ_GET_RX_SPEED		0xF0 // Get IR receiving speed
#define LTW_REQ_SET_RX_SPEED		0xF1 // Set IR receiving speed
#define LTW_REQ_GET_TX_STATE		0xF2 // Get state of the transmitter 
#define LTW_REQ_GET_TX_CARRIER_FREQUENCY  0xF3	// Set transmission carrier frequency 
#define LTW_REQ_SET_TX_CARRIER_FREQUENCY  0xF4	// Set transmission carrier frequency 
#define LTW_REQ_GET_TX_CARRIER_DUTY_CYCLE 0xF5	// Set transmission carrier dutycycle
#define LTW_REQ_SET_TX_CARRIER_DUTY_CYCLE 0xF6	// Set transmission carrier dutycycle
#define LTW_REQ_GET_CAPS			0xFC // Get capabilities
#define LTW_REQ_GET_VERSION			0xFD // Get version information
#define LTW_REQ_GET_COPYRIGHT		0xFE // Get copyright information
#define LTW_REQ_GET_CREDITS			0xFF // Credits list

//	Request return codes (bLtwErrCode in the reply format) 
//	
#define LTW_REQERR_SUCCESS			0x00 // Request succeded
#define LTW_REQERR_BADPARM			0x01 // Bad vendparameter and/or value
#define LTW_REQERR_BUSY				0x02 // Tower is busy
#define LTW_REQERR_NOPOWER			0x03 // Not enough power (current) to carry out the requested operation
#define LTW_REQERR_WRONGMODE		0x04 // Not in the right mode to execute this request
#define LTW_INTERNAL_ERROR			0xFE // Internal error in the Tower
#define LTW_REQERR_BADREQUEST		0xFF // Bad request

// Bad request reply format
//
typedef struct LTW_REQ_BAD_REPLY
{
	Word wNoOfBytes;			// Number of bytes in the reply
	Byte bErrCode;			// Request return code
	Byte bSpare;				// Used only for having a even number of bytes in the reply
} LTW_REQ_BAD_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_PARM and SET_PARM
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Parameter (1 byte) and value (1 byte) for GET_PARM and SET_PARM request
//	
//	The parameter code goes in the low-order byte of wvalue. the value (argument)
//	for the request goes into the high-order byte. windex is reserved for future 
//	use.
//	

#define LTW_PARM_MODE       	0x01			// Tower mode:
#define LTW_MODE_VLL			0x01		// - Send VLL
#define LTW_MODE_IR				0x02		// - Send IR
#define LTW_MODE_IRC			0x04		// Send/Receive IR according to the LEGO Infrared Remote Control protocol (Technic Cars)
#define LTW_MODE_RADIO			0x08		// Send/Receive Radio signals


#define LTW_PARM_RANGE			0x02	        // Transmission range:                                                                                     
#define LTW_RANGE_SHORT			0x01		// - Short
#define LTW_RANGE_MEDIUM		0x02		// - Medium
#define LTW_RANGE_LONG			0x03		// - Long

#define LTW_PARM_ERRDETECT		0x03			// Error detection on IR receiver:
#define LTW_ERRDETECT_ON		0x01		// - on
#define LTW_ERRDETECT_OFF		0x02		// - off

#define LTW_PARM_ERRSTATUS		0x04			// Current internal errorcode:
#define LTW_NO_ERROR					0	// - No error
// Function/parameter error
#define LTW_NO_SUPPORT					1	// - No support for this function
#define LTW_ERR_VEND_REQ			   10	// - Invalid vendor request
// USB errors
#define LTW_ERR_USB					  100	// - Unspecified USB problem
#define LTW_ERR_USB_FUNC			  101	// - Call to wrong USB function
#define LTW_ERR_USB_DATA			  102	// - No or invalid data received
#define LTW_ERR_USB_ENDPOINT		  103	// - Wrong endpoint
#define LTW_ERR_USB_PACKETSIZE		  104	// - Invalid packetsize		
#define LTW_ERR_USB_TX_EP0			  105	// - Error on sending on endpoint 0
#define LTW_ERR_USB_EP0_PACKETSIZE	  106	// - Tried to send more than MAXPACKETSIZE on endpoint 0
#define LTW_ERR_USB_EP1_PACKETSIZE	  107	// - Tried to send more than MAXPACKETSIZE on endpoint 1
#define LTW_ERR_USB_VEND_BUFF_FULL	  108	// - Vendor answer buffer full
#define LTW_ERR_USB_TX_EP1			  109   // - Error on sending on endpoint 1

// UART errors
#define	LTW_ERR_UART				  200	// - UART problems
#define LTW_ERR_UART_MODE			  201	// - Invalid UART mode		
#define LTW_ERR_UART_TX_STATE		  210	// - Invalid TX state
#define LTW_ERR_UART_LOW_POWER		  211   // - Tried to send in long range when configured as a low power device	
// Tower errors
#define LTW_ERR_TOWER				  240	// - Tower out of order
#define LTW_ERR_UNDEFINED			  255	// - undefined error		


#define LTW_PARM_ENDIAN			0x97		// Endian (word-format):
#define LTW_ENDIAN_LITTLE		0x01	// - Little endian (Intel word format: LSB first, MSB last)	
#define LTW_ENDIAN_BIG			0x02	// - Big endian (Others word format: MSB first, LSB last)

#define LTW_PARM_ID_LED_MODE	0x98		// Indicator LED control mode:
#define LTW_ID_LED_HW_CTRL		0x01	// - Indicator LED is controlled by the firmware	
#define LTW_ID_LED_SW_CTRL		0x02	// - Indicator LED is controlled by the host software (see REQ_SET_IDLED)

#define LTW_PARM_ERROR_SIGNAL	0x99		// Signal on internal error:
#define LTW_ID_LED_ON			0x01	// - Indicator LED will be switched on when a serious error occurs	
#define LTW_ID_LED_OFF			0x02	// - Indicator LED will NOT be switched on when a serious error occurs

// --------------------------------------------------------------
// Reply format
//	
//	All vendor request replies from the tower start with the following
//	four bytes:
//	

typedef struct LTW_REQ_REPLY_HEADER
{
	Word wNoOfBytes;						// Number of bytes in the reply
	Byte bErrCode;						// Request return code
	Byte bValue;							// Request return value
} LTW_REQ_REPLY_HEADER;

typedef LTW_REQ_REPLY_HEADER LTW_REQ_GET_SET_PARM_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: SET_PARM_IRC and GET_PARM_IRC
////////////////////////////////////////////////////////////////////////////////////////////////////

#define LTW_PARM_IRC_PACKETSIZE       	0x01	// Packet size, in bytes (default = 2)
#define LTW_PARM_IRC_TIMEFRAME       	0x02	// Time frame for packet syncronization, in ms (default = 80)

typedef LTW_REQ_REPLY_HEADER LTW_REQ_GET_SET_PARM_IRC_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: FLUSH
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Combined parameter and value in 2 bytes for FLUSH request
//	
//  Format: 1. byte = Buffer no. as they are defined under SET_LED request, 2. byte = 00
//	-------------------------------------------------------------
//	
//	Parameter: Buffer no.	This goes into the low-order byte of the wvalue field of the 
//							request's setup packet
//	
#define LTW_TX_BUFFER		0x01		// Transmission buffer 
#define LTW_RX_BUFFER		0x02		// Receiver buffer 
#define LTW_ALL_BUFFERS		0x03		// All buffers 


//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_FLUSH_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bBufferno;					// Flushed bufferno.
} LTW_REQ_FLUSH_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: RESET
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_RESET_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes
} LTW_REQ_RESET_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_STAT
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_STAT_REPLY
{
	Word  wNoOfBytes;					// Number of bytes in the reply
	Byte  bErrCode;					// Request return code
	Byte  bSpare;						// Spare byte to obtain an even number of bytes

	Word  wNoOfRxBytes;				// Number of received bytes
	Word  wOverrunErrorCount;			// # of times overrun error has happen
	Word  wNoiseCount;				// # of bytes with wrong bits
	Word  wFramingErrorCount;			// # of bytes with framing errors 
} LTW_REQ_GET_STAT_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: GET_POWER 
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_POWER_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bPower;						// Powermode (POWER_LOW or POWER_HIGH)
#define LTW_POWER_LOW			0x01    // - Tower is set to low power device
#define LTW_POWER_HIGH			0x02    // - Tower is set to high power device
	Byte bSpare2;

} LTW_REQ_GET_POWER_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: SET_LED and GET_LED
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Parameter (1 byte) and value (1 byte) for SET_LED request
//	
//	Parameter: Led no.	This goes into the low-order byte of the wvalue field of the 
//						request's setup packet
//	
#define LTW_LED_ID					0x01 // ID LED 
#define LTW_LED_VLL					0x02 // VLL LED 

//	Value:     Led color	THis goes into the high-order byte of the wvalue field of the 
//							request's setup packet
//
//	COLOR identifiers (low order bytes in the value field of the request)
//	if a color not supported by the hardware is used, the firmware approximates 
//	to the nearest one. The current USB Tower version supports only Green color.
//	
#define LTW_LED_COLOR_BLACK			0x00 // Black = same as off
#define LTW_LED_COLOR_GREEN			0x01 // Green
#define LTW_LED_COLOR_YELLOW		0x02 // Yellow
#define LTW_LED_COLOR_ORANGE		0x04 // Orange
#define LTW_LED_COLOR_RED			0x08 // Red
//	
//	more color codes can be added here...
//												
#define LTW_LED_COLOR_DEFAULT		0xFF		// Whatever color is the default

#define LTW_LED_COLOR_ON			LTW_LED_COLOR_DEFAULT		// generic ON
#define LTW_LED_COLOR_OFF			LTW_LED_COLOR_BLACK			// generic OFF


//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_SET_LED_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bLedId;						// See parameter 
	Byte bColor;						// See value 
} LTW_REQ_GET_SET_LED_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: RESET_STAT
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_RESET_STAT_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Used only for having a even number of bytes in the reply
} LTW_REQ_RESET_STAT_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_PARM_IRC and SET_PARM_IRC
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Parameter (1 byte) and value (1 byte) for GET_PARM_IRC and SET_PARM_IRC request
//	
//	The parameter code goes in the low-order byte of wvalue. the value (argument)
//	for the request goes into the high-order byte. windex is reserved for future 
//	use.
//	

#define LTW_PARM_IRC_PACKETSIZE 0x01			// IRC packetsize
// Value: Packetsize in bytes

#define LTW_PARM_IRC_DELAY_TX	0x02	        // Delay between packets to send                                                                                   
// Value: Transmit delay time between packets in ms.

// --------------------------------------------------------------
// Reply format
//	
typedef LTW_REQ_REPLY_HEADER LTW_REQ_GET_SET_PARM_IRC_REPLY;



////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: GET and SET_TX_SPEED
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Combined parameter and value in 2 bytes for GET and SET_TX_SPEED request
//	
#define SPEED_COMM_BAUD_1200			CAPS_COMM_BAUD_1200
#define SPEED_COMM_BAUD_2400			CAPS_COMM_BAUD_2400		
#define SPEED_COMM_BAUD_4800			CAPS_COMM_BAUD_4800
#define SPEED_COMM_BAUD_9600			CAPS_COMM_BAUD_9600
#define SPEED_COMM_BAUD_19200			CAPS_COMM_BAUD_19200

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_SET_TX_SPEED_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Word wTxSpeed;					// Transmission speed 
} LTW_REQ_GET_SET_TX_SPEED_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: GET and SET_RX_SPEED
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Combined parameter and value in 2 bytes for GET and SET_RX_SPEED request
//
//  Same definitions as SET_TX_SPEED !	

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_SET_RX_SPEED_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Word wRxSpeed;					// Receiving speed 
} LTW_REQ_GET_SET_RX_SPEED_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: GET_TX_STATE
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_TX_STATE_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bTxState;					// Transmitter states:
#define CAPS_TX_STATE_READY		0x01	// - Transmitter buffer empty, Ready to receive more data to send
#define CAPS_TX_STATE_BUSY		0x02	// - Busy sending data
	Byte bSpare2;						// Spare byte to obtain an even number of bytes
} LTW_REQ_GET_TX_STATE_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: SET_TX_CARRIER_FREQUENCY 
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Combined parameter and value in 2 bytes for SET_TX_CARRIER_FREQUENCY request
//	
//  Parameter (1 byte):  frequency e.g.: 38 kHz = 38
//  Value     (1 bytes): 00

//	The parameter code goes in the low-order byte of wvalue. the value (argument)
//	for the request goes into the high-order byte. windex is reserved for future 
//	use.

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_SET_TX_CARRIER_FREQUENCY_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bTxFrequency;				// Transmission frequency
	Byte bSpare2;						// Spare byte to obtain an even number of bytes
} LTW_REQ_GET_SET_TX_CARRIER_FREQUENCY_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUESTS: SET_TX_CARRIER_DUTY_CYCLE 
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Parameter (1 byte) and value (1 byte) for SET_TX_CARRIER_DUTY_CYCLE request
//	
//	Parameter: Range		This goes into the low-order byte of the wvalue field of the 
//							request's setup packet
//	
//    See LTW_PARM_RANGE for definitions.
//

//	Value:     Duty cycle	This goes into the high-order byte of the wvalue field of the 
//							request's setup packet
//
//	  Duty cycle in us multiplied by 10. E.g. 1,5 us = 15

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_SET_TX_CARRIER_DUTY_CYCLE_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bTxRange;					// Transmission range
	Byte bSpare2;

	Word wTxDutyCycle;				// Transmission dutycycle in this range	in 1/10 of a ms.	
} LTW_REQ_GET_SET_TX_CARRIER_DUTY_CYCLE_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_CAPS
////////////////////////////////////////////////////////////////////////////////////////////////////

//	-------------------------------------------------------------
//	Combined parameter and value in 2 bytes for GET_CAPS request
//	

//	Supported communication modes
#define LTW_CAPS_VLL				LTW_MODE_VLL	
#define LTW_CAPS_IR					LTW_MODE_IR	
#define LTW_CAPS_IRC				LTW_MODE_IRC
#define LTW_CAPS_RADIO				LTW_MODE_RADIO

//	-------------------------------------------------------------
//  Reply format
//
typedef struct LTW_REQ_GET_CAPS_REPLY
{
	Word wNoOfBytes;							// Number of bytes in the reply
	Byte bErrCode;							// Request return code
	Byte bSpare;								// Spare byte to obtain even number of bytes

	Byte bCommDirections;						// Communication directions:
#define CAPS_COMM_DIRECTION_TRANSMIT	0x01	// - Transmit
#define CAPS_COMM_DIRECTION_RECEIVE		0x02	// - Receive
#define CAPS_COMM_DIRECTION_BOTH		0x03	// - Both transmit and receive
	Byte bCommRange;							// Communication range:
#define CAPS_COMM_RANGE_SHORT			0x01	// - Short 
#define CAPS_COMM_RANGE_MEDIUM			0x02	// - Medium
#define CAPS_COMM_RANGE_LONG			0x04	// - Long
#define CAPS_COMM_RANGE_ALL				0x07	// - Both short, medium and long range
	Word wCommTransmitRate;					// Communication transmit rate (same definition as receive rate)
	Word wCommReceiveRate;					// Communication receive rate:
#define CAPS_COMM_BAUD_1200				0x0004  // - 1200 baud
#define CAPS_COMM_BAUD_2400				0x0008	// - 2400 baud
#define CAPS_COMM_BAUD_4800				0x0010	// - 4800 baud
#define CAPS_COMM_BAUD_9600				0x0020	// - 9600 baud
#define CAPS_COMM_BAUD_19200			0x0040	// - 19200 baud
	Byte bCommTransmitMinFrequency;			// Minimum frequency between 30 and 99 kHz. E.g. 30 kHz = 30
	Byte bCommTransmitMaxFrequency;			// Maximum frequency between 30 and 99 kHz. E.g. 30 kHz = 30
	Word wCommTransmitMinDutyCycle;			// Minimum duty cycle in us multiplied by 100. E.g.  1,25 us = 125
	Word wCommTransmitMaxDutyCycle;			// Maximum duty cycle in us multiplied by 100. E.g. 18,9  us = 1890
	Byte bNoOfBytesInUartTxBuffer;			// Number of bytes defined in the transmitter buffer
	Byte bNoOfBytesInUartRxBuffer;			// Number of bytes defined in the receiver buffer
} LTW_REQ_GET_CAPS_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_VERSION
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
#pragma pack(push, 1)
typedef struct LTW_REQ_GET_VERSION_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	Byte bMajorVersion;				// Major version e.g. 1
	Byte bMinorVersion;				// Minor version e.g. 01
	Word wBuildNo;					// Build no. e.g. 0112
} LTW_REQ_GET_VERSION_REPLY;
#pragma pack(pop)


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_COPYRIGHT
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_COPYRIGHT_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	// Copyright string: placed in the Tower ROM -- NOT null-terminated
} LTW_REQ_GET_COPYRIGHT_REPLY;


////////////////////////////////////////////////////////////////////////////////////////////////////
//                      VENDOR REQUEST: GET_CREDITS
////////////////////////////////////////////////////////////////////////////////////////////////////

//	No request parameter or value
//

//	-------------------------------------------------------------
// Reply format
//
typedef struct LTW_REQ_GET_CREDITS_REPLY
{
	Word wNoOfBytes;					// Number of bytes in the reply
	Byte bErrCode;					// Request return code
	Byte bSpare;						// Spare byte to obtain an even number of bytes

	// Credits string: placed in the Tower ROM -- NOT null-terminated
} LTW_REQ_GET_CREDITS_REPLY;

#endif