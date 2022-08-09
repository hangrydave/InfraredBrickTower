#include "../pch.h"

//	
//	PBKTYPES.H
//	
//	Basic type definitions for PBK Library (aka Ghost)
//	
//	Written by AS
//	Copyright (C) 2000 The LEGO Company. All rights reserved.
//


#if defined(_MSC_VER) && (_MSC_VER > 1000)
	#pragma once
#endif

#ifndef __PBKTYPES_H__
#define __PBKTYPES_H__

	//	======================================================================
	//	BASIC TYPES DEFINITIONS
	//	

	//	Here starts the Autoduck commenting style
	//	@doc

	//	@type uchar  | unsigned char
	typedef unsigned char uchar;

	//	@type uint8  | 8-bit unsigned integer (same as uchar)
	typedef unsigned char uint8;

	//	@type int8  | 8-bit signed integer
	typedef signed char int8;

	//	@type uint32 | unsigned long (32 bits)
	typedef unsigned long uint32;

	//	@type int32  | 32-bit signed value
	typedef signed long int32;

	//	@type uint16 | unsigned short (16 bits)
	typedef unsigned short uint16;

	//	@type int16  | 16-bit signed value
	typedef signed short int16;

	//	@type uint   | unsigned int
	typedef unsigned int uint;
		
	//	@type PBKRESULT			| The general result type for the PBrick Comm API
	typedef long PBKRESULT;

#endif	// PBKTYPES