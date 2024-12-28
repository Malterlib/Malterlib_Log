// Copyright © 2023 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/CommandLine/AnsiEncoding>

namespace NMib::NLog
{
#if DMibSysLogSeverities
	struct CLogToStdErrAnsi
	{
		CLogToStdErrAnsi
			(
				NCommandLine::EAnsiEncodingFlag _AnsiFlags
				, NLog::ESeverity _Severities
				, bool _bTrace
				, mint _CategoryWidth = fg_GetSys()->f_GetEnvironmentVariable("MalterlibLogCategoryWidth", "32").f_ToInt(mint(32))
				, mint _SeverityWidth = fg_GetSys()->f_GetEnvironmentVariable("MalterlibLogSeverityWidth", "10").f_ToInt(mint(10))
			)
		;

		void operator()
			(
				mint _ThreadID
				, NTime::CTime const &_Time
				, NLog::ESeverity _Sev
				, NLog::CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, NLog::CLogLocationTag const& _Loc
			)
		;

		NStr::CStrNonTracked f_FormatLog
			(
				mint _ThreadID
				, NTime::CTime const &_Time
				, NLog::ESeverity _Sev
				, NLog::CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, NLog::CLogLocationTag const& _Loc
			)
		;

	private:
		NCommandLine::CAnsiEncoding mp_AnsiEncoding;
		NLog::ESeverity mp_Severities = NLog::ESeverity_All;
		NStr::CStr mp_EmptyColor;
		NStr::CStr mp_TimeColor;
		NStr::CStr mp_CategoryColor;
		NStr::CStr mp_DebugColor;
		NStr::CStr mp_StdErrColor;
		NStr::CStr mp_StdOutColor;
		NStr::CStr mp_LogColor;
		NStr::CStr mp_CriticalColor;
		NStr::CStr mp_Indent;

		mint mp_CategoryWidth = 32;
		mint mp_SeverityWidth = 10;

		bool mp_bTrace = false;
	};
#endif
}
