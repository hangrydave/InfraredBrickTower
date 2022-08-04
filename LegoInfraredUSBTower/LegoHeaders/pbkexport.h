#include "../pch.h"

//	
//	X-Compiler and X-Platform 
//	definitions for function exporting and calling conventions	
//	
//	
//	-----------------------------------------------------------------
//	
//	Written by AS
//	Copyright (C) 2001 The LEGO Company. All rights reserved.
//
//	

#ifndef __PBKEXPORT_H__

	#define __PBKEXPORT_H__

	#if defined(__MACINTOSH__) || defined(macintosh)  		// MetroWerks
		#ifndef __MACINTOSH__
			#define __MACINTOSH__
		#endif
	#endif
	#if defined (_WIN32)		// VC++ and Metrowerks
								// Insert here constants for other Win compilers e.g. "|| defined(__WINDOWS__) \"
		#ifndef _WIN32
			#define _WIN32
		#endif
	#endif

					
	//	-----------------------------------------------
	//	
	//	MSVC-specific definitions
	//	
	#if defined(_MSC_VER) && (_MSC_VER > 1000)

		//	Definition for the stdcall ("pascal") calling convention. 
		//	
		#define PBK_STDCALL         __stdcall

		//
		//	The following is needed in order to export the PBrick comm classes
		//	from the Win32 DLL. The exported classes will have the PBKCOMM_EXPORT
		//	modifier (defined as nothing if not using MSVC)
		//

		#if (defined (PBKCOMM_EXPORTING)) || (defined(PBK_EXPORTING))
			#define PBKCOMM_EXPORT	__declspec(dllexport)	// Obsolete -- use only for backward compatibility
			#define PBK_EXPORT		__declspec(dllexport)
		#else
			#define PBKCOMM_EXPORT	__declspec(dllimport)	// Obsolete -- use only for backward compatibility
			#define PBK_EXPORT		__declspec(dllimport)
		#endif

	//	-----------------------------------------------
	//	
	//	Metrowerks' CodeWarrior
	//	
	#elif defined(__MWERKS__)

		//	CW seems to have different keywords for Windows and Mac compilers
		//	
		//	See MSVC-specific for a description of the constants

		#if defined(_WIN32)

			#define PBK_STDCALL         __stdcall

			#if (defined (PBKCOMM_EXPORTING)) || (defined(PBK_EXPORTING))
				#define PBKCOMM_EXPORT	__declspec(dllexport)	// Obsolete -- use only for backward compatibility
				#define PBK_EXPORT		__declspec(dllexport)
			#else
				#define PBKCOMM_EXPORT	__declspec(dllimport)	// Obsolete -- use only for backward compatibility
				#define PBK_EXPORT		__declspec(dllimport)
			#endif

		#elif defined(__MACINTOSH__)	// predefined constant for the Mac targets according to MW docs

			#define PBK_STDCALL         

			#if (defined (PBKCOMM_EXPORTING)) || (defined(PBK_EXPORTING))
				#define PBKCOMM_EXPORT	__declspec(export)	// Obsolete -- use only for backward compatibility
				#define PBK_EXPORT		__declspec(export)
			#else
				#define PBKCOMM_EXPORT	__declspec(import)	// Obsolete -- use only for backward compatibility
				#define PBK_EXPORT		__declspec(import)
			#endif

		//	
		// #elif
		// add here for additional platforms supported by the MW compiler	
		//	
		#else
			#error ERROR: Only Mac or Win32 targets supported at the moment!
		#endif

	//	
	// #elif
	// add here for additional compiler
	//	

    #else

		//	Generic compiler -- you may want to generate an 
		//	error instead of dummy definitions:

		//	
		// #error ERROR: Please define the correct PBK_STDCALL and PBK_EXPORT values for this compiler
		//	

        #define PBK_STDCALL

		#define PBKCOMM_EXPORT		// Obsolete -- use only for backward compatibility
		#define PBK_EXPORT 

    #endif

#endif	//	 __PBKEXPORT_H__
