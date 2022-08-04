#include "../pch.h"

//	
//	PBKERROR.H
//	
//	Error codes and macro definitions for the PBrick communication stack
//	
//	Written by AS
//	Copyright (C) 1998 The LEGO Group. All rights reserved.
//


#if defined(_MSC_VER) && (_MSC_VER > 1000)
	#pragma once
#endif

#ifndef __PBKERROR_H__
#define __PBKERROR_H__

//	@doc   RETURNCODES
//	
//	@topic Error Codes		|
//	PBrick Communication API Error code definitions. Most functions in the PBrick communication
//	API return a value of type PBKRESULT. This value can indicate a success or an error condition.<nl>
//	<nl>
//	
//	The Error code format used is Win32-HRESULT compatible, but it should work fine
//	on any other platform as well. The format is defined as follows:<nl>
//	<nl>
//	
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1<nl>
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0<nl>
//  +---+-+-+-----------------------+-------------------------------+<nl>
//  :Sev:C:R:     Facility          :               Code            :<nl>
//  +---+-+-+-----------------------+-------------------------------+<nl>
//<nl><nl>
//	
//  where:<nl>
//	<nl>
//  Sev - is the severity code:<nl>
//	<nl>
//      <tab>00 - Success			<nl>
//      <tab>01 - Informational		<nl>
//      <tab>10 - Warning			<nl>
//      <tab>11 - Error				<nl>
//	<nl>
//	<nl>
//	C - Custom bit. Always 1 for Windows compatibility reasons<nl>
//	<nl>
//	<nl>
//	R - Reserved by NT. Always 0.
//	<nl>
//	<nl>
//	Facility - Facility code. It identifies the component this error belongs to.<nl>
//	<nl>
//	Code - Actual Error code.<nl>
//	<nl>
//	
//	The COM HRESULT codes will be compatible with this format, with the difference
//	that Warnings will be treated like errors, and informational msg will be treated
//	as success (HRESULT format has only 1 severity bit).<nl>
//	<nl>
//	
//	

#define PBKERR_SEVERITYBITS			  0xC0000000

#define PBKERR_SEVERITY_SUCCESS       0x00000000
#define PBKERR_SEVERITY_INFORMATIONAL 0x40000000
#define PBKERR_SEVERITY_WARNING       0x80000000
#define PBKERR_SEVERITY_ERROR         0xC0000000

#define PBKERR_CUSTOM			      0x20000000

#define FAC_PBK						  0x01000000		// error codes generated in the Ghost communication stack (session, protocol, port)
#define FAC_USBTOWER				  0x02000000		// error codes from the USB Tower firmware are translated into Win32 errors using this facility 

//	
//	
#define PBKERR_MASK						(PBKERR_CUSTOM | FAC_PBK )
#define PBKERR_MASK_FAC(fac)			(PBKERR_CUSTOM | fac )

#define PBKERR_MAKE( sev, fac, code )	( sev | PBKERR_MASK_FAC(fac) | code )
#define PBKERR_MAKE_ERROR( fac, code )	(PBKERR_MAKE( PBKERR_SEVERITY_ERROR, fac , code ) )
#define PBKERR_MAKE_WARNING( fac, code )(PBKERR_MAKE( PBKERR_SEVERITY_WARNING, PBKERR_MASK_FAC(fac), code ) )
#define PBKERR_MAKE_INFO( fac, code )	(PBKERR_MAKE( PBKERR_SEVERITY_INFORMATIONAL, PBKERR_MASK_FAC(fac), code ) )
#define PBKERR_MAKE_SUCCESS( fac, code )(PBKERR_MAKE( PBKERR_SEVERITY_SUCCESS, PBKERR_MASK_FAC(fac), code ) )

#define PBKERR_MASK_SUCCESS				(PBKERR_SEVERITY_SUCCESS		| PBKERR_MASK)
#define PBKERR_MASK_INFO				(PBKERR_SEVERITY_INFORMATIONAL	| PBKERR_MASK)
#define PBKERR_MASK_WARNING				(PBKERR_SEVERITY_WARNING		| PBKERR_MASK)
#define PBKERR_MASK_ERROR				(PBKERR_SEVERITY_ERROR			| PBKERR_MASK)

#define PBK_IS_SUCCESS(e)				((e&PBKERR_SEVERITYBITS)==PBKERR_SEVERITY_SUCCESS)
#define PBK_IS_INFO(e)					((e&PBKERR_SEVERITYBITS)==PBKERR_SEVERITY_INFORMATIONAL)
#define PBK_IS_WARNING(e)				((e&PBKERR_SEVERITYBITS)==PBKERR_SEVERITY_WARNING)
#define PBK_IS_ERROR(e)					((e&PBKERR_SEVERITYBITS)==PBKERR_SEVERITY_ERROR)

inline bool PBK_SUCCEEDED(PBKRESULT e)	{ return (PBK_IS_SUCCESS(e) || PBK_IS_INFO(e) );	}
inline bool PBK_FAILED(PBKRESULT e)		{ return (PBK_IS_ERROR(e)	|| PBK_IS_WARNING(e) );	}

//	@enum	Success Results		|	Success codes. Return codes for which the PBK_IS_SUCCESS macro
//									returns TRUE
//	
enum {

	PBKOK			= 0,								//	@emem	No error occurred
	PBKOK_PENDING	= PBKERR_MASK_SUCCESS | 0x0001		//	@emem	Command pending

};

//	@enum	Informational Results	|	Another kind of success codes. The ones for which the PBK_IS_INFO macro
//										returns TRUE
//	
enum {

	PBK_INFO_DUMMY	= PBKERR_MASK_INFO	| 0x0000			//	@emem	Dummy informational result
};


//	@enum	Informational Results	|	Warning results. The ones for which the PBK_IS_WARNING macro
//										returns TRUE
//	
enum {

	PBK_WNG_DUMMY	= PBKERR_MASK_WARNING	| 0x0000			//	@emem	Dummy warning result
};


//	@const	|	PBKOK	|	No error occurred
#define PBKOK		0


//	@enum	Error Results	|	Error codes. The ones for which the PBK_IS_ERROR macro
//								returns TRUE. The following errors use the facility FAC_PBK
//	
enum {

	//	Generic errors
	//	

	PBK_ERR_GENERIC			= PBKERR_MASK_ERROR	| 0x0000,			//	@emem	Generic error
	PBK_ERR_BADPARM			= PBKERR_MASK_ERROR	| 0x0001,			//	@emem	Bad parameters
	PBK_ERR_NOTIMPLEMENTED	= PBKERR_MASK_ERROR	| 0x0002,			//	@emem	Method not implemented
	PBK_ERR_NOMEMORY		= PBKERR_MASK_ERROR	| 0x0003,			//	@emem	Insufficient memory to perform the requested operation
	PBK_ERR_OVERFLOW		= PBKERR_MASK_ERROR	| 0x0004,			//	@emem	Memory overflow - buffer size insufficient to hold requested data

	PBK_ERR_ALREADYOPEN		= PBKERR_MASK_ERROR	| 0x0101,	//	@emem	Port (or protocol, or session) already open
	PBK_ERR_CLOSED			= PBKERR_MASK_ERROR	| 0x0102,	//	@emem	Port (or protocol, or session) already closed
	PBK_ERR_BUSY			= PBKERR_MASK_ERROR	| 0x0103,	//	@emem	Requested service is incompatible with the current state
	PBK_ERR_NOMORE			= PBKERR_MASK_ERROR	| 0x0104,	//	@emem	No more items or devices could be found
	PBK_ERR_TIMEOUT			= PBKERR_MASK_ERROR	| 0x0105,	//	@emem	Communication timeout occurred. Couldn't complete the requested operation
	PBK_ERR_INVALIDSTATE	= PBKERR_MASK_ERROR	| 0x0106,	//	@emem	The object is in an invalid state
	PBK_ERR_NOTFOUND		= PBKERR_MASK_ERROR	| 0x0107,	//	@emem	Item not found
	PBK_ERR_ABORT			= PBKERR_MASK_ERROR	| 0x0108,	//	@emem	Abort requested for the specified command

	PBK_ERR_OPEN			= PBKERR_MASK_ERROR	| 0x0110,	//	@emem	Error opening port (or protocol, or session)
	PBK_ERR_SEND			= PBKERR_MASK_ERROR	| 0x0111,	//	@emem	Transmission error
	PBK_ERR_RECV			= PBKERR_MASK_ERROR	| 0x0112,	//	@emem	Receive error
	PBK_ERR_DEVICECONTROL	= PBKERR_MASK_ERROR	| 0x0113,	//	@emem	Error in <om IPbkPort::devicecontrol>
	PBK_ERR_GETTHROUGHPUT	= PBKERR_MASK_ERROR	| 0x0114,	//	@emem	Error in <om IPbkPort::getthroughput>
	PBK_ERR_SETTIMEOUT		= PBKERR_MASK_ERROR	| 0x0115,	//	@emem	Error in <om IPbkPort::settimeout>
	PBK_ERR_GETTIMEOUT		= PBKERR_MASK_ERROR	| 0x0116,	//	@emem	Error in <om IPbkPort::gettimeout>
	PBK_ERR_CREATETHREAD	= PBKERR_MASK_ERROR	| 0x0117,	//	@emem	Error while creating a thread
	PBK_ERR_NOPORTSET		= PBKERR_MASK_ERROR	| 0x0118,	//	@emem	No port object was bound to protocol object
	PBK_ERR_NOPROTOCOLSET	= PBKERR_MASK_ERROR	| 0x0119,	//	@emem	No protocol object was bound to session object
	PBK_ERR_CLEAR			= PBKERR_MASK_ERROR	| 0x011A,	//	@emem	Error in <om IPbkPort::flush>
	PBK_ERR_CANTABORT		= PBKERR_MASK_ERROR	| 0x011B,	//	@emem	Error in <om IPbkPort::abort>
	PBK_ERR_SENDCOMMAND		= PBKERR_MASK_ERROR	| 0x011C,	//	@emem	Error in <om IPbkProtocol::sendcommand>
	PBK_ERR_DISCONNECTED	= PBKERR_MASK_ERROR	| 0x011D,	//	@emem	Device has been disconnected or other unrecoverable error
	PBK_ERR_INUSE			= PBKERR_MASK_ERROR	| 0x011E,	//	@emem	Resource is already in use.

	//	Protocol-specific errors
	//	

	PBK_ERR_PROT_BADHEADER		= PBKERR_MASK_ERROR	| 0x0201,	//	@emem	Bad or missing protocol packet header
	PBK_ERR_PROT_BADCHECKSUM	= PBKERR_MASK_ERROR	| 0x0202,	//	@emem	Bad or protocol packet checksum
	PBK_ERR_PROT_BADPACKETLEN	= PBKERR_MASK_ERROR	| 0x0203,	//	@emem	Bad packet length
	PBK_ERR_PROT_EXCEEDRETRIES	= PBKERR_MASK_ERROR	| 0x0204,	//	@emem	Retry count exceeded
	PBK_ERR_PROT_NACK			= PBKERR_MASK_ERROR	| 0x0205,	//	@emem	Negative acknowledge received
	PBK_ERR_PROT_STATSDISABLED	= PBKERR_MASK_ERROR	| 0x0206,	//	@emem	Statistics are currently disabled

	//	Session-specific errors
	//	
	PBK_ERR_SESS_NOREQUEST			= PBKERR_MASK_ERROR	| 0x0301,	//	@emem	Request queue empty

	PBK_ERR_ABORT_ALREADYREQUESTED	= PBKERR_MASK_ERROR	| 0x0302,	//	@emem	Abort already requested for the specified PBK_HREQUEST
	PBK_ERR_DIAG_NOTOWER			= PBKERR_MASK_ERROR	| 0x0340	//	@emem	No tower seems to be connected to the currently open protocol/port

};


//	@enum	USB Tower Firmware Error Results	|	The following error codes (the integer part) are the ones used internally by
//													the USB Tower firmware. They are translated into WIn32 format 
//													by the TowerAPI functions, using the facility FAC_USBTOWER.<nl>
//													<nl>
//													Applications wishing to communicate directly with the USB Tower
//													driver/firmware, should then convert those error codes into
//													Win32 format by calling LEGOTowerGetWin32Error( tower error )
//	
enum {

	PBK_ERR_USB_SUCCESS				= PBKOK,

	PBK_ERR_USB_NOTIMPLEMENTED		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 1 ),		// @emem	Unsupported request
	PBK_ERR_USB_INVALIDREQUEST		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 10),		// @emem	Invalid request

	// USB errors
	PBK_ERR_USB_GENERIC				= PBKERR_MAKE_ERROR( FAC_USBTOWER, 100),		// @emem	Unspecified USB problem
	PBK_ERR_USB_INVALIDFUNCTION		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 101),		// @emem	Call to wrong USB function
	PBK_ERR_USB_INVALIDDATA			= PBKERR_MAKE_ERROR( FAC_USBTOWER, 102),		// @emem	No or invalid data received
	PBK_ERR_USB_INVALIDENDPOINT		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 103),		// @emem	Invalid endpoint
	PBK_ERR_USB_INVALIDPACKETSIZE	= PBKERR_MAKE_ERROR( FAC_USBTOWER, 104),		// @emem	Invalid packetsize		
	PBK_ERR_USB_TX_EP0				= PBKERR_MAKE_ERROR( FAC_USBTOWER, 105),		// @emem	Error on sending on endpoint 0
	PBK_ERR_USB_EP0_PACKETSIZE		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 106),		// @emem	Tried to send more than MAXPACKETSIZE on endpoint 0
	PBK_ERR_USB_EP1_PACKETSIZE		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 107),		// @emem	Tried to send more than MAXPACKETSIZE on endpoint 1
	PBK_ERR_USB_BUFFERFULL			= PBKERR_MAKE_ERROR( FAC_USBTOWER, 108),		// @emem	Answer buffer full

	// UART (IR) errors
	PBK_ERR_USB_UART				= PBKERR_MAKE_ERROR( FAC_USBTOWER, 200),		// @emem	SW UART problems
	PBK_ERR_USB_INVALIDMODE			= PBKERR_MAKE_ERROR( FAC_USBTOWER, 201),		// @emem	Invalid UART mode		
	PBK_ERR_USB_INVALIDSTATE		= PBKERR_MAKE_ERROR( FAC_USBTOWER, 210),		// @emem	Invalid TX state	

	// Tower errors
	PBK_ERR_USB_TOWER				= PBKERR_MAKE_ERROR( FAC_USBTOWER, 240),		// @emem	Tower out of order
	PBK_ERR_USB_UNDEFINED			= PBKERR_MAKE_ERROR( FAC_USBTOWER, 255)			// @emem	undefined error		

};

//	Error boundaries (for debug checking)
//	

#define USBTOWER_ERR_MIN		1
#define USBTOWER_ERR_MAX		255
	
#endif //__PBKERROR_H__