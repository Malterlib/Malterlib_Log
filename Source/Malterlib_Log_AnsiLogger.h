// Copyright © 2023 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/CommandLine/AnsiEncoding>

namespace NMib::NLog
{
#if DMibSysLogSeverities
	struct CLogToStdErrAnsi
	{
		CLogToStdErrAnsi(NCommandLine::EAnsiEncodingFlag _AnsiFlags, NLog::ESeverity _Severities, bool _bTrace);
		
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

	private:
		NCommandLine::CAnsiEncoding mp_AnsiEncoding;
		NLog::ESeverity mp_Severities = NLog::ESeverity_All;
		NStr::CStr mp_EmptyColor;
		NStr::CStr mp_TimeColor;
		NStr::CStr mp_CategoryColor;
		NStr::CStr mp_DebugColor;
		bool mp_bTrace = false;
	};
#endif
}
