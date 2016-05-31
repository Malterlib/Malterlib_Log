// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once
#include <Mib/Core/Core>

namespace NMib
{

	namespace NLog
	{

#if DMibSysLogSeverities

		void fg_LogTo_DebugOut
			(
				mint _ThreadID
				, NTime::CTime const& _Time
				, ESeverity _Sev
				, CLogStr const& _Message
				, DMibListLinkDS_List(CSysLogCatScope, m_Link) const &_Categories
				, DMibListLinkDS_List(CSysLogOpScope, m_Link) const &_Operations
				, CLogLocationTag const& _Loc
			)
		;

		void fg_LogTo_StdErr
			(
				mint _ThreadID
				, NTime::CTime const& _Time
				, ESeverity _Sev
				, CLogStr const& _Message
				, DMibListLinkDS_List(CSysLogCatScope, m_Link) const &_Categories
				, DMibListLinkDS_List(CSysLogOpScope, m_Link) const &_Operations
				, CLogLocationTag const& _Loc
			)
		;

		struct CLogFile
		{			
			CLogStr m_Filename;
			CLogStr m_Directory;
			bint m_bFilenameUsedTime;
			NFile::CFile m_File;
			NThread::CMutual m_Lock;

			CLogFile();
			~CLogFile();
			bint f_ReadyForWrite();
			
			void f_PrepareFork();
			void f_ForkedChild();
			void f_ForkedParent();
		};

		struct CFileLogger
		{
			CFileLogger(CLogFile *_pLogFile);
			
			void operator()
				(
					mint _ThreadID
					, NTime::CTime const& _Time
					, ESeverity _Sev
					, CLogStr const& _Message
					, DMibListLinkDS_List(CSysLogCatScope, m_Link) const &_Categories
					, DMibListLinkDS_List(CSysLogOpScope, m_Link) const &_Operations
					, CLogLocationTag const& _Loc
				)
			;
		
		private:
			CLogFile *mp_pLogFile;
		};

		struct CLogToFile
		{
		private:
			CLogger &mp_Logger;
			CLogFile mp_File;

		public:

			CLogToFile(CLogger& _Logger, CLogStr const& _File)
				: mp_Logger(_Logger)
			{
				mp_File.m_Filename = _File;
				mp_Logger.f_PushDestination(CFileLogger(&mp_File));
			}

			CLogToFile(CLogger& _Logger, CLogStr const& _File, CLogFilter&& _Filter)
				: mp_Logger(_Logger)
			{
				mp_File.m_Filename = _File;
				mp_Logger.f_PushDestination(CFileLogger(&mp_File), fg_Move(_Filter));
			}

			~CLogToFile()
			{
				mp_Logger.f_PopDestination();
			}
		};

		#define DMibLogToFile(_File) NMib::NLog::CLogToFile l_LogToFile##__LINE__(NMib::fg_GetSys()->f_GetLogger(), _File)
		#define DMibLogToFileEx(_Tag, _File) NMib::NLog::CLogToFile l_LogToFile##__LINE__##_Tag(NMib::fg_GetSys()->f_GetLogger(), _File)
		#define DMibLogToFileFiltered(_File, _Filter) NMib::NLog::CLogToFile l_LogToFile##__LINE__(NMib::fg_GetSys()->f_GetLogger(), _File, _Filter)
		#define DMibLogToFileFilteredEx(_Tag, _File, _Filter) NMib::NLog::CLogToFile l_LogToFile##__LINE__##_Tag(NMib::fg_GetSys()->f_GetLogger(), _File, _Filter)

#else
		#define DMibLogToFile(_File) (void)0
		#define DMibLogToFileEx(_Tag, _File) (void)0
		#define DMibLogToFileFiltered(_File, _Filter) (void)0
		#define DMibLogToFileFilteredEx(_Tag, _File, _Filter) (void)0
#endif

		#ifndef DMibPNoShortCuts
			#define DLogToFile(_File) DMibLogToFile(_File)
			#define DLogToFileEx(_Tag, _File) DMibLogToFileEx(_Tag, _File)
			#define DLogToFileFiltered(_File, _Filter) DMibLogToFileFiltered(_File, _Filter)
			#define DLogToFileFilteredEx(_Tag, _File, _Filter) DMibLogToFileFilteredEx(_Tag, _File, _Filter)
		#endif

	} // Namespace NLog

} // Namespace NMib
