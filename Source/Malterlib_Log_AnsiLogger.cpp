// Copyright © 2023 Favro Holding AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Log_AnsiLogger.h"

#include <Mib/CommandLine/AnsiEncodingParse>

namespace NMib::NLog
{
#if (DMibSysLogSeverities) != 0

	static constexpr NStr::CStr gc_Indent = NStr::gc_Str<"                                                                     ">;

	CLogToStdErrAnsi::CLogToStdErrAnsi(NCommandLine::EAnsiEncodingFlag _AnsiFlags, NLog::ESeverity _Severities, bool _bTrace, mint _CategoryWidth, mint _SeverityWidth)
		: mp_AnsiEncoding(_AnsiFlags)
		, mp_Severities(_Severities)
		, mp_bTrace(_bTrace)
		, mp_CategoryWidth(fg_Max(_CategoryWidth, 5u))
		, mp_SeverityWidth(fg_Max(_SeverityWidth, 5u))
	{
		using namespace NStr;

		mp_TimeColor = mp_AnsiEncoding.f_ForegroundRGB(128, 128, 128);
		mp_DebugColor = mp_AnsiEncoding.f_ForegroundRGB(100, 100, 100);
		mp_CategoryColor = mp_AnsiEncoding.f_ForegroundRGB(51, 182, 255);
		mp_StdErrColor = mp_AnsiEncoding.f_ForegroundRGB(0xffb680);
		mp_LogColor = mp_AnsiEncoding.f_ForegroundRGB(0xffd700);
		mp_StdOutColor = mp_AnsiEncoding.f_ForegroundRGB(0xdbd3ff);
		mp_CriticalColor = mp_AnsiEncoding.f_Bold() + mp_AnsiEncoding.f_ForegroundRGB(0xff3f1c);

		if (mp_SeverityWidth + mp_CategoryWidth == 42)
			mp_Indent = gc_Indent;
		else
			mp_Indent = "{sf ,sj*}"_f << "" << (gc_Indent.f_GetLen() + (aint(mp_SeverityWidth) - 10) + (aint(mp_CategoryWidth) - 32));
	}

	[[maybe_unused]] static NStr::CUStr fg_ShortenStringMiddle(NStr::CUStr const &_String, mint _MaxLen)
	{
		using namespace NStr;

		if (_String.f_GetLen() <= aint(_MaxLen))
			return _String;

		mint LeftLen = _MaxLen / 2;
		mint RightLen = (_MaxLen - LeftLen - 1);

		return CUStr::CFormat(str_utf32("{}…{}")) << _String.f_Left(LeftLen) << _String.f_Right(RightLen);
	}

	NStr::CStrNonTracked CLogToStdErrAnsi::f_FormatLog
		(
			mint _ThreadID
			, NTime::CTime const &_Time
			, NLog::ESeverity _Sev
			, NLog::CLogStr const &_Message
			, NContainer::TCVector<NStr::CStr> const &_Categories
			, NContainer::TCVector<NStr::CStr> const &_Operations
			, NLog::CLogLocationTag const &_Loc
		)
	{
		if ((_Sev & mp_Severities) == NLog::ESeverity_None)
			return {};

		NTime::CTimeConvert::CDateTime DateTime;
		NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

		if (!_Operations.f_IsEmpty() && _Operations.f_GetFirst() == NStr::gc_Str<"DisableStdErrLogger">.m_Str)
			return {};

		bool bStdOut = false;
		bool bLog = false;
		bool bStdErr = false;

		auto SeverityString = [&]() -> NStr::CUStr
			{
				if (mp_AnsiEncoding.f_Color())
				{
					if (!_Operations.f_IsEmpty() && _Operations.f_GetFirst() != NStr::gc_Str<"DisableDistributedLogReporter">.m_Str)
					{
						bLog = _Operations.f_GetFirst() == NStr::gc_Str<"Log">.m_Str;
						bStdOut = _Operations.f_GetFirst() == NStr::gc_Str<"StdOut">.m_Str;
						bStdErr = _Operations.f_GetFirst() == NStr::gc_Str<"StdErr">.m_Str;
						return _Operations.f_GetFirst();
					}
					else
						return NLog::fg_GetSeverityName(_Sev);
				}
				else
				{
					if (!_Operations.f_IsEmpty() && _Operations.f_GetFirst() != NStr::gc_Str<"DisableDistributedLogReporter">.m_Str)
						return NStr::CUStr::CFormat(str_utf32("[{}]")) << _Operations.f_GetFirst();
					else
						return NStr::CUStr::CFormat(str_utf32("[{}]")) << NLog::fg_GetSeverityName(_Sev);
				}
			}
			()
		;

		SeverityString = fg_ShortenStringMiddle(SeverityString, mp_SeverityWidth);

		mint SeverityOffset = fg_Max((mp_SeverityWidth - SeverityString.f_GetLen()) / 2, 0u);

		auto CategoryString = [&]() -> NStr::CUStr
			{
				if (_Categories.f_IsEmpty())
					return {};

				if (mp_AnsiEncoding.f_Color())
					return _Categories.f_GetFirst();

				return NStr::CUStr::CFormat(str_utf32("<{}>")) << _Categories.f_GetFirst();
			}
			()
		;

		CategoryString = fg_ShortenStringMiddle(CategoryString, mp_CategoryWidth);

		return NStr::CUStr
			(
				NStr::CUStr::CFormat
				(
					str_utf32("{}{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}.{fr1,fe3}{}  {}{sj*}{} {}{sj*,a-*}{} {}{\n}")
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
				<< (bLog ? mp_LogColor : mp_CategoryColor)
				<< CategoryString
				<< mp_CategoryWidth
				<< mp_AnsiEncoding.f_Default()
				<< [&]() -> NStr::CStr const &
				{
					switch(_Sev)
					{
					case NLog::ESeverity_None:
					case NLog::ESeverity_Info:
						if (bStdOut || bLog)
							return mp_StdOutColor;
						else
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
					case NLog::ESeverity_Error:
					case NLog::ESeverity_Perf_Error:
						if (bStdErr)
							return mp_StdErrColor;
						else
							return mp_AnsiEncoding.f_StatusError();
					case NLog::ESeverity_Critical:
							return mp_CriticalColor;
					case NLog::ESeverity_All:
						DMibNeverGetHere;
					}

					return mp_AnsiEncoding.f_Default();
				}
				()
				<< SeverityString
				<< mp_SeverityWidth
				<< SeverityOffset
				<< mp_AnsiEncoding.f_Default()
				<< _Message.f_Indent(mp_Indent, false)
			)
		;
	}

	void CLogToStdErrAnsi::operator()
		(
			mint _ThreadID
			, NTime::CTime const &_Time
			, NLog::ESeverity _Sev
			, NLog::CLogStr const &_Message
			, NContainer::TCVector<NStr::CStr> const &_Categories
			, NContainer::TCVector<NStr::CStr> const &_Operations
			, NLog::CLogLocationTag const &_Loc
		)
	{
		auto LogString = f_FormatLog(_ThreadID, _Time, _Sev, _Message, _Categories, _Operations, _Loc);
		if (!LogString)
			return;

		if (mp_bTrace)
			DMibTraceRaw(LogString.f_GetStr());
		else
			DMibConErrOutRaw(LogString.f_GetStr());
	}
#endif
}
