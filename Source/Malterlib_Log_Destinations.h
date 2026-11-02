// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once
#include <Mib/Core/Core>

namespace NMib::NLog
{
#if DMibSysLogSeverities

	FLogDestination fg_LogTo_DebugOut();
	FLogDestination fg_LogTo_StdErr();

	struct CLogFile
	{
		CLogStr m_Filename;
		CLogStr m_Directory;
		bool m_bFilenameUsedTime;
		NFile::CFile m_File;
		NThread::CMutual m_Lock;

		CLogFile();
		~CLogFile();
		bool f_ReadyForWrite();

		void f_PrepareFork();
		void f_ForkedChild();
		void f_ForkedParent();
	};

	struct CFileLogger
	{
		CFileLogger(CLogFile *_pLogFile);

		void operator ()
			(
				umint _ThreadID
				, NTime::CTime const &_Time
				, ESeverity _Sev
				, CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const &_Loc
			)
		;

	private:
		CLogFile *mp_pLogFile;
	};

	struct CLogToFile : public CCoroutineThreadLocalHandler
	{
	private:
		CLogger &mp_Logger;
		CLogFile mp_File;
		CLogFilter mp_Filter;

	public:
		CLogToFile(CLogger &_Logger, CLogStr const &_File)
			: mp_Logger(_Logger)
		{
			mp_File.m_Filename = _File;
			mp_Logger.f_PushDestination(CFileLogger(&mp_File));
		}

		CLogToFile(CLogger &_Logger, CLogStr const &_File, CLogFilter &&_Filter)
			: mp_Logger(_Logger)
			, mp_Filter(fg_Move(_Filter))
		{
			mp_File.m_Filename = _File;
			mp_Logger.f_PushDestination(CFileLogger(&mp_File), mp_Filter);
		}

		~CLogToFile()
		{
			mp_Logger.f_PopDestination();
		}

		void f_Suspend() noexcept override
		{
			mp_Logger.f_PopDestination();
		}

		void f_ResumeNoExcept() noexcept override
		{
			mp_Logger.f_PushDestination(CFileLogger(&mp_File), mp_Filter);
		}
	};

#	define DMibLogToFile(d_File) NMib::NLog::CLogToFile l_LogToFile##__LINE__(NMib::fg_GetSys()->f_GetLogger(), d_File)
#	define DMibLogToFileEx(d_Tag, d_File) NMib::NLog::CLogToFile l_LogToFile##__LINE__##d_Tag(NMib::fg_GetSys()->f_GetLogger(), d_File)
#	define DMibLogToFileFiltered(d_File, d_Filter) NMib::NLog::CLogToFile l_LogToFile##__LINE__(NMib::fg_GetSys()->f_GetLogger(), d_File, d_Filter)
#	define DMibLogToFileFilteredEx(d_Tag, d_File, d_Filter) NMib::NLog::CLogToFile l_LogToFile##__LINE__##d_Tag(NMib::fg_GetSys()->f_GetLogger(), d_File, d_Filter)

#else
#	define DMibLogToFile(d_File) (void)0
#	define DMibLogToFileEx(d_Tag, d_File) (void)0
#	define DMibLogToFileFiltered(d_File, d_Filter) (void)0
#	define DMibLogToFileFilteredEx(d_Tag, d_File, d_Filter) (void)0
#endif

#ifndef DMibPNoShortCuts
#	define DLogToFile(d_File) DMibLogToFile(d_File)
#	define DLogToFileEx(d_Tag, d_File) DMibLogToFileEx(d_Tag, d_File)
#	define DLogToFileFiltered(d_File, d_Filter) DMibLogToFileFiltered(d_File, d_Filter)
#	define DLogToFileFilteredEx(d_Tag, d_File, d_Filter) DMibLogToFileFilteredEx(d_Tag, d_File, d_Filter)
#endif
}

#ifndef DMibPNoShortCuts
using namespace NMib::NLog;
#endif
