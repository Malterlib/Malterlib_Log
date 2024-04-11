// Copyright © 2023 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Log_AnsiLogger.h"

namespace NMib::NLog
{
#if (DMibSysLogSeverities) != 0

	static constexpr NStr::CStr gc_Indent = NStr::gc_Str<"                                                                     ">;

	CLogToStdErrAnsi::CLogToStdErrAnsi(NCommandLine::EAnsiEncodingFlag _AnsiFlags, NLog::ESeverity _Severities, bool _bTrace)
		: mp_AnsiEncoding(_AnsiFlags)
		, mp_Severities(_Severities)
		, mp_bTrace(_bTrace)
	{
		mp_TimeColor = mp_AnsiEncoding.f_ForegroundRGB(128, 128, 128);
		mp_DebugColor = mp_AnsiEncoding.f_ForegroundRGB(100, 100, 100);
		mp_CategoryColor = mp_AnsiEncoding.f_ForegroundRGB(51, 182, 255);
	}

	void CLogToStdErrAnsi::operator()
		(
			mint _ThreadID
			, NTime::CTime const &_Time
			, NLog::ESeverity _Sev
			, NLog::CLogStr const &_Message
			, NContainer::TCVector<NStr::CStr> const &_Categories
			, NContainer::TCVector<NStr::CStr> const &_Operations
			, NLog::CLogLocationTag const& _Loc
		)
	{
		if ((_Sev & mp_Severities) == NLog::ESeverity_None)
			return;

		NTime::CTimeConvert::CDateTime DateTime;
		NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

		if (!_Operations.f_IsEmpty() && _Operations.f_GetFirst() == "DisableStdErrLogger")
			return;

		auto SeverityString = [&]() -> NStr::CStr
			{
				if (mp_AnsiEncoding.f_Color())
				{
					if (!_Operations.f_IsEmpty() && _Operations.f_GetFirst() != "DisableDistributedLogReporter")
						return _Operations.f_GetFirst();
					else
						return NLog::fg_GetSeverityName(_Sev);
				}
				else
				{
					if (!_Operations.f_IsEmpty())
						return NStr::fg_Format("[{}]", _Operations.f_GetFirst());
					else
						return NStr::fg_Format("[{}]", NLog::fg_GetSeverityName(_Sev));
				}
			}
			()
		;

		mint SeverityOffset = fg_Max((10 - SeverityString.f_GetLen()) / 2, 0);

		auto CategoryString = [&]() -> NStr::CStr
			{
				if (_Categories.f_IsEmpty())
					return {};

				if (mp_AnsiEncoding.f_Color())
					return _Categories.f_GetFirst();

				return NStr::fg_Format("<{}>", _Categories.f_GetFirst());
			}
			()
		;

		NStr::CStrNonTracked OutputString = NStr::CStrNonTracked::CFormat
			(
				"{}{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}.{fr1,fe3}{}  {}{sj32}{} {}{sj10,a-*}{} {}{\n}"
			)
			<< mp_TimeColor
			<< DateTime.m_Year
			<< DateTime.m_Month
			<< DateTime.m_DayOfMonth
			<< DateTime.m_Hour
			<< DateTime.m_Minute
			<< DateTime.m_Second
			<< DateTime.m_Fraction
			<< mp_AnsiEncoding.f_Default()
			<< mp_CategoryColor
			<< CategoryString
			<< mp_AnsiEncoding.f_Default()
			<< [&]() -> NStr::CStr const &
			{
				switch(_Sev)
				{
				case NLog::ESeverity_None:
				case NLog::ESeverity_Info:
					return mp_AnsiEncoding.f_StatusNormal();
				case NLog::ESeverity_Debug:
				case NLog::ESeverity_DebugVerbose1:
				case NLog::ESeverity_DebugVerbose2:
				case NLog::ESeverity_DebugVerbose3:
				case NLog::ESeverity_Perf_Info:
					return mp_DebugColor;
				case NLog::ESeverity_Warning:
				case NLog::ESeverity_Perf_Warning:
					return mp_AnsiEncoding.f_StatusWarning();
				case NLog::ESeverity_Critical:
				case NLog::ESeverity_Error:
				case NLog::ESeverity_Perf_Error:
					return mp_AnsiEncoding.f_StatusError();
				case NLog::ESeverity_All:
					DMibNeverGetHere;
				}

				return mp_AnsiEncoding.f_Default();
			}
			()
			<< SeverityString
			<< SeverityOffset
			<< mp_AnsiEncoding.f_Default()
			<< _Message.f_Indent(gc_Indent, false)
		;

		if (mp_bTrace)
			DMibTraceRaw(OutputString.f_GetStr());
		else
			DMibConErrOutRaw(OutputString.f_GetStr());
	}
#endif
}
