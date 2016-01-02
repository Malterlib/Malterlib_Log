// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

/*
	Malterlib Logging Configuration Macros

*/
#pragma once

#define DMibLogSeverity_None		0
#define DMibLogSeverity_Debug		1
#define DMibLogSeverity_Info		2
#define DMibLogSeverity_Warning		4
#define DMibLogSeverity_Error		8
#define DMibLogSeverity_Perf_Info	16
#define DMibLogSeverity_Perf_Warning	32
#define DMibLogSeverity_Perf_Error	64
#define DMibLogSeverity_Critical	128
#define DMibLogSeverity_All			(1 | 2 | 4 | 8 | 16 | 32 | 64 | 128)

// Define DMibSysLogSeverities to specify which severities to log.
#ifndef DMibSysLogSeverities

#if defined(DConfig_Release) || defined(DConfig_Optimized)
	#define DMibSysLogSeverities DMibLogSeverity_None
#elif defined(DConfig_ReleaseTesting)
	#define DMibSysLogSeverities (DMibLogSeverity_Error | DMibLogSeverity_Critical | DMibLogSeverity_Warning | DMibLogSeverity_Info | DMibLogSeverity_Debug)
#elif defined(DConfig_Profile)
	#define DMibSysLogSeverities (DMibLogSeverity_Error | DMibLogSeverity_Perf_Info | DMibLogSeverity_Perf_Warning | DMibLogSeverity_Perf_Error | DMibLogSeverity_Critical)
#elif defined(DMibDebug)
	#define DMibSysLogSeverities (DMibLogSeverity_All & ~(DMibLogSeverity_Perf_Info | DMibLogSeverity_Perf_Warning | DMibLogSeverity_Perf_Error) )
#else
	#define DMibSysLogSeverities (DMibLogSeverity_Warning | DMibLogSeverity_Error | DMibLogSeverity_Critical)
#endif

#endif

// Currently Unsupported:
	/*

// Define DMibSysLogFile0_* to specify the first log file to use, and which severities to include.
#if defined(DConfig_Release)
//	#define DMibSysLogFile0_File "DMib_Log.txt"
//	#define DMibSysLogFile0_Filter DMibLogSeverity_All
#else
	#define DMibSysLogFile0_File "DMib_Log.txt"
	#define DMibSysLogFile0_Filter DMibLogSeverity_All
#endif

// Define DMibSysLogFile1_* to specify the second log file to use, and which severities to include.
#if defined(DConfig_Release)
//	#define DMibSysLogFile1_File "DMib_Log_Errors.txt"
//	#define DMibSysLogFile1_Filter (DMibLogSeverity_Error | DMibLogSeverity_Critical)
#else
	#define DMibSysLogFile1_File "DMib_Log_Errors.txt"
	#define DMibSysLogFile1_Filter (DMibLogSeverity_Error | DMibLogSeverity_Critical)
#endif

// Define DMibSysLogFile2_* to specify the second log file to use, and which severities to include.
#if defined(DConfig_Profile)
	#define DMibSysLogFile2_File "DMib_Log_Perf.txt"
	#define DMibSysLogFile2_Filter (DMibLogSeverity_Perf_Info | DMibLogSeverity_Perf_Warning | DMibLogSeverity_Perf_Error)
#else
//	#define DMibSysLogFile2_File "DMib_Log_Perf.txt"
//	#define DMibSysLogFile2_Filter (DMibLogSeverity_Perf_Info | DMibLogSeverity_Perf_Warning | DMibLogSeverity_Perf_Error)	
#endif
*/
	