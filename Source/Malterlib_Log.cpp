// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Function/Function>
#include "Malterlib_Log_Configuration.h"
#include "Malterlib_Log_Destinations.h"
#include "Malterlib_Log.h"

namespace NMib
{

	namespace NLog
	{
		using namespace NFile;

#if DMibSysLogSeverities

		CNullLogger::CNullLogger()
		{
		}
		CNullLogger::~CNullLogger()
		{
		}

		bint CNullLogger::f_ReadConfig(CLogStr const& _Path) 
		{ 
			return false; 
		}

		mint CNullLogger::f_PushGlobalDestination(FLogDestination &&_fLog) 
		{ 
			return 0; 
		}
		
		mint CNullLogger::f_PushGlobalDestination(FLogDestination &&_fLog, CLogFilter&& _Filter) 
		{ 
			return 0; 
		}
		
		bool CNullLogger::f_PopGlobalDestination() 
		{ 
			 return true; 
		}
		
		void CNullLogger::f_RemoveGlobalDestination(mint _ID)
		{
		}

		void CNullLogger::f_PushDestination(FLogDestination &&_fLog) 
		{
		}
		
		void CNullLogger::f_PushDestination(FLogDestination &&_fLog, CLogFilter&& _Filter) 
		{
		}
		
		void CNullLogger::f_PopDestination() 
		{
		}

		void CNullLogger::f_PushCategoryScope(CSysLogCatScope &_Scope)
		{
		}
		
		void CNullLogger::f_PopCategoryScope(CSysLogCatScope &_Scope)
		{
		}

		void CNullLogger::f_PushOperationScope(CSysLogOpScope &_Scope)
		{
		}
		
		void CNullLogger::f_PopOperationScope(CSysLogOpScope &_Scope)
		{
		}

		void CNullLogger::f_Log(CLogLocationTag _Loc, ESeverity _Sev, CLogStr const& _Str)
		{
		}

		void CNullLogger::f_Submit(ESeverity _Sev, CLogStr&& _Text)
		{
		}

		void CNullLogger::f_PushCategoryScope(NStr::CStr const& _Str)
		{
		}
		
		void CNullLogger::f_PushCategoryScope(NStr::CWStr const& _Str)
		{
		}
		
		void CNullLogger::f_PushCategoryScope(NStr::CUStr const& _Str)
		{
		}

		void CNullLogger::f_PushOperationScope(NStr::CStr const& _Str)
		{
		}
		
		void CNullLogger::f_PushOperationScope(NStr::CWStr const& _Str)
		{
		}
		
		void CNullLogger::f_PushOperationScope(NStr::CUStr const& _Str)
		{
		}

#if 0
		static char const* fg_ExtractFileName(char const* _pPath)
		{
			if (!_pPath)
				return nullptr;

			mint Len = NStr::fg_StrLen(_pPath);

			if (!Len)
				return nullptr;

			char const* pPtr = _pPath + (Len - 1);
			char const* pEnd = _pPath - 1;

			while(pPtr > pEnd)
			{
				char Ch = *pPtr;
				if (	Ch == '/'
					||	Ch == '\\')
					return pPtr +1 ;
				--pPtr;
			}

			return _pPath;
		}
#endif

		struct CLogger::CDetails
		{
			struct CDestination
			{
				FLogDestination m_fLog;
				CLogFilter m_Filter;
			};

			struct CThreadInfo
			{
				NContainer::TCVector<NStr::CStr> f_GetCategoryScopeStack() const;
				NContainer::TCVector<NStr::CStr> f_GetOperationScopeStack() const;
				
				DMibListLinkDS_List(CSysLogCatScope, m_Link) m_CategoryStack;
				DMibListLinkDS_List(CSysLogOpScope, m_Link) m_OperationStack;
				NContainer::TCVector<NPtr::TCSharedPointer<CDestination, NMem::CAllocator_NonTrackedHeap>, NMem::CAllocator_NonTrackedHeap> m_lDestinations;
			};

			NThread::TCThreadLocal<CThreadInfo, NMem::CAllocator_NonTrackedHeap> mp_ThreadInfo;

			NThread::CMutualManyRead mp_GlobalDestLock; // TODO: Do without? Require global dests set at startup?
			mint mp_NextGlobalDestinationID;
			NContainer::TCMap<mint, NPtr::TCSharedPointer<CDestination, NMem::CAllocator_NonTrackedHeap>, CSort_Default, NMem::CAllocator_NonTrackedHeap> mp_GlobalDestinations;

			NContainer::TCVector< NPtr::TCUniquePointer<CLogFile> > mp_lConfigLogFiles;
			
			FLogDispatch mp_Dispatcher;
		};

		CLogger::CLogger()
		{
			mp_pD = fg_Construct();
			DMibCheck(true); // Add dependency to check system
		}

		CLogger::~CLogger()
		{
		}
		
		void CLogger::f_PrepareFork()
		{
			mp_pD->mp_GlobalDestLock.f_Lock();
			mp_pD->mp_GlobalDestLock.f_PrepareFork();
		}
		
		void CLogger::f_ForkedChild()
		{
			mp_pD->mp_GlobalDestLock.f_ForkedChild();
			mp_pD->mp_GlobalDestLock.f_Unlock();
		}
		
		void CLogger::f_ForkedParent()
		{
			mp_pD->mp_GlobalDestLock.f_ForkedParent();
			mp_pD->mp_GlobalDestLock.f_Unlock();
		}		

		bint CLogger::f_ReadConfig(CLogStr const& _Path)
		{
			CLogStr Config;

			try
			{
				if (CFile::fs_FileExists(_Path))
				{
					Config = CFile::fs_ReadStringFromFile(_Path);
				}
				else
				{
					CLogStr Path = CLogStr::CFormat("{}/{}") << fg_GetSys()->f_GetProgramRootNonTracked() << _Path;
					if (CFile::fs_FileExists(Path))
						Config = CFile::fs_ReadStringFromFile(Path);
					else
					{
						CLogStr Path2 = CLogStr::CFormat("{}/{}") << CFile::fs_GetProgramDirectoryNonTracked() << _Path;
						if (CFile::fs_FileExists(Path2))
							Config = CFile::fs_ReadStringFromFile(Path2);
						else
						{
							return false;
						}
					}
				}
			}
			catch (CExceptionFile const &)
			{
				return false;
			}

			CLogStr CurLine;
			CLogStr::CChar const* pCurPos = Config.f_GetStr();
			while(pCurPos && *pCurPos != 0)
			{
				NStr::fg_ParseWhiteSpace(pCurPos);
				CLogStr::CChar const* pLineEnd = pCurPos;
				NStr::fg_ParseToEndOfLine(pLineEnd);

				CurLine = CLogStr(pCurPos, pLineEnd - pCurPos);

				{
					CLogStr Dest = NStr::fg_GetStrSep(CurLine, ":");

					NContainer::TCVector<CLogStr> lArgs;
					while(!CurLine.f_IsEmpty())
					{
						lArgs.f_Insert( NStr::fg_GetStrSep(CurLine, ",").f_Trim() );
					}

					fp_AddGlobalDestination(Dest, lArgs);

				}

				pCurPos = pLineEnd;
			}


			return true;
		}

		void CLogger::fp_AddGlobalDestination(CLogStr const& _Name, NContainer::TCVector<CLogStr> const& _lArgs)
		{
			auto fl_ParseFilter = 
				[](NContainer::TCVector<CLogStr> const& _lArgs, mint _iFirst, CLogFilter& _oFilter)
				{

					auto fl_ReadArg	=
						[&]() -> CLogStr
						{
							if (_iFirst < _lArgs.f_GetLen())
							{
								return _lArgs[_iFirst++];
							}
							else
								return CLogStr();
						};

					_oFilter.m_Severity = ESeverity_None;

					{ // Severity Filter
						CLogStr Severities = fl_ReadArg();

						while(!Severities.f_IsEmpty())
						{
							CLogStr Sev = fg_GetStrSep(Severities, "|").f_Trim();

							_oFilter.m_Severity |= fg_LookupSeverity(Sev);							
						}
					}

					{ // Category Filter
						CLogStr Category = fl_ReadArg();
						if (!Category.f_IsEmpty())
						{
							_oFilter.m_Category = Category;
						}
					}

					{ // Operation Filter
						CLogStr Operation = fl_ReadArg();
						if (!Operation.f_IsEmpty())
						{
							_oFilter.m_Operation = Operation;
						}
					}

					{ // File Filter
						CLogStr File = fl_ReadArg();
						if (!File.f_IsEmpty())
						{
							_oFilter.m_File = File;
						}
					}
				};

			CLogFilter Filter;

			if (_Name.f_CmpNoCase("DebugOut") == 0)
			{
				fl_ParseFilter(_lArgs, 0, Filter);
				f_PushGlobalDestination(&fg_LogTo_DebugOut, fg_Move(Filter));
			}
			else if (_Name.f_CmpNoCase("File") == 0)
			{
				if (_lArgs.f_IsEmpty())
					return;

				CLogStr LogFile = _lArgs[0];
				fl_ParseFilter(_lArgs, 1, Filter);

				NPtr::TCUniquePointer<CLogFile> pLogFile = fg_Construct();

				pLogFile->m_Filename = LogFile;

				f_PushGlobalDestination(CFileLogger(pLogFile.f_Get()), fg_Move(Filter));

				mp_pD->mp_lConfigLogFiles.f_Insert(fg_Move(pLogFile));
			}
		}

		mint CLogger::f_PushGlobalDestination(FLogDestination &&_fLog)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			mint ID = ++mp_pD->mp_NextGlobalDestinationID;
			CDetails::CDestination& NewDest = *(mp_pD->mp_GlobalDestinations[ID] = fg_Construct());
			NewDest.m_fLog = fg_Move(_fLog);
			return ID;
		}

		void CLogger::f_RemoveGlobalDestination(mint _DestinationID)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			mp_pD->mp_GlobalDestinations.f_Remove(_DestinationID);
		}

		mint CLogger::f_PushGlobalDestination(FLogDestination &&_fLog, CLogFilter &&_Filter)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			mint ID = ++mp_pD->mp_NextGlobalDestinationID;
			CDetails::CDestination &NewDest = *(mp_pD->mp_GlobalDestinations[ID] = fg_Construct());
			NewDest.m_fLog = fg_Move(_fLog);
			NewDest.m_Filter = fg_Move(_Filter);
			return ID;
		}

		bool CLogger::f_PopGlobalDestination()
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			if (mp_pD->mp_GlobalDestinations.f_IsEmpty())
				return false;
			mp_pD->mp_GlobalDestinations.f_Remove(mp_pD->mp_GlobalDestinations.f_FindLargest());
			return !mp_pD->mp_GlobalDestinations.f_IsEmpty();
		}

		void CLogger::f_PushDestination(FLogDestination &&_fLog)
		{
			CDetails::CDestination& NewDest = *((*mp_pD->mp_ThreadInfo).m_lDestinations.f_Insert() = fg_Construct());
			NewDest.m_fLog = fg_Move(_fLog);
		}

		void CLogger::f_PushDestination(FLogDestination &&_fLog, CLogFilter&& _Filter)
		{
			CDetails::CDestination& NewDest = *((*mp_pD->mp_ThreadInfo).m_lDestinations.f_Insert() = fg_Construct());
			NewDest.m_fLog = fg_Move(_fLog);
			NewDest.m_Filter = fg_Move(_Filter);
		}

		void CLogger::f_PopDestination()
		{
			(*mp_pD->mp_ThreadInfo).m_lDestinations.f_Pop();
		}

		void CLogger::f_PushCategoryScope(CSysLogCatScope &_Scope)
		{
			(*mp_pD->mp_ThreadInfo).m_CategoryStack.f_InsertFirst(_Scope);
		}

		void CLogger::f_PopCategoryScope(CSysLogCatScope &_Scope)
		{
			(*mp_pD->mp_ThreadInfo).m_CategoryStack.f_Remove(_Scope);
		}

		void CLogger::f_PushOperationScope(CSysLogOpScope &_Scope)
		{
			(*mp_pD->mp_ThreadInfo).m_OperationStack.f_InsertFirst(_Scope);
		}

		void CLogger::f_PopOperationScope(CSysLogOpScope &_Scope)
		{
			(*mp_pD->mp_ThreadInfo).m_OperationStack.f_Remove(_Scope);
		}

		NContainer::TCVector<NStr::CStr> CLogger::CDetails::CThreadInfo::f_GetCategoryScopeStack() const
		{
			NContainer::TCVector<NStr::CStr> Categories;
			for (auto &Category : m_CategoryStack)
				Categories.f_Insert(Category.m_pCategory);
			return Categories;
		}
		
		NContainer::TCVector<NStr::CStr> CLogger::CDetails::CThreadInfo::f_GetOperationScopeStack() const
		{
			NContainer::TCVector<NStr::CStr> Operations;
			for (auto &Operation : m_OperationStack)
				Operations.f_Insert(Operation.m_pOperation);
			return Operations; 
		}
		
		void CLogger::f_SetDispatcher(FLogDispatch &&_fDispatcher)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			mp_pD->mp_Dispatcher = fg_Move(_fDispatcher);
		}

		void CLogger::f_Log(CLogLocationTag _Loc, ESeverity _Sev, CLogStr const& _Text)
		{
			NTime::CTime LogTime = NTime::CTime::fs_NowUTC();

			mint ThreadID = NSys::fg_Thread_GetCurrentUID();
			
			auto &Details = *mp_pD;
			
			auto &ThreadInfo = *Details.mp_ThreadInfo;

			auto fSendToDests =
				[&](auto &_Container, auto const &_Categories, auto const &_Operations)
				{
					for (auto &pDestination : _Container)
					{
						auto &Destination = *pDestination;
						if 
							(
								Destination.m_Filter.f_Test
								(
									ThreadID
									, LogTime
									, _Sev
									, _Text
									, _Categories
									, _Operations
									, _Loc
								)
							)
						{
							if (Details.mp_Dispatcher)
							{
								Details.mp_Dispatcher
									(
										[pDestination, ThreadID, LogTime, _Sev, _Text, _Categories, _Operations, _Loc]() mutable
										{
											pDestination->m_fLog
												( 
													ThreadID
													, LogTime
													, _Sev
													, _Text
													, _Categories
													, _Operations
													, _Loc
												)
											;
										}
									)
								;
							}
							else
							{
								Destination.m_fLog
									( 
										ThreadID
										, LogTime
										, _Sev
										, _Text
										, _Categories
										, _Operations
										, _Loc
									)
								;
							}
						}
					}
				}
			;
			{
				DMibLockRead(Details.mp_GlobalDestLock);
				if (ThreadInfo.m_lDestinations.f_IsEmpty() && Details.mp_GlobalDestinations.f_IsEmpty())
					return;
				
				auto Cats = ThreadInfo.f_GetCategoryScopeStack();
				auto Ops = ThreadInfo.f_GetOperationScopeStack();

				fSendToDests(ThreadInfo.m_lDestinations, Cats, Ops);
				fSendToDests(Details.mp_GlobalDestinations, Cats, Ops);
			}
		}

		// CLogFilter

		bint CLogFilter::f_Test(
				mint _ThreadID
			,	NTime::CTime const& _Time
			,	ESeverity _Sev
			, 	CLogStr const& _Message
			,	NContainer::TCVector<NStr::CStr> const &_Categories
			,	NContainer::TCVector<NStr::CStr> const &_Operations
			,	CLogLocationTag const& _Loc
			)
		{
			if (m_Severity != ESeverity_None && (_Sev & m_Severity) == 0)
				return false;

			if (!m_Category.f_IsEmpty())
			{
				bool bFound = false;
				for (auto &Category : _Categories)
				{
					if (m_Category.f_CmpNoCase(Category) == 0)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					return false;
			}

			if (!m_Operation.f_IsEmpty())
			{
				bool bFound = false;
				for (auto &Operation : _Operations)
				{
					if (m_Operation.f_CmpNoCase(Operation) == 0)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					return false;
			}

			if (!m_File.f_IsEmpty())
			{
				mint FileLen = m_File.f_GetLen();
				mint LocFileLen = NStr::fg_StrLen(_Loc.m_pFile);

				if (FileLen > LocFileLen)
					return false;

				mint nOffset = LocFileLen - FileLen;

				if (NStr::fg_StrCmpNoCase(m_File.f_GetStr(), _Loc.m_pFile + nOffset) != 0)
					return false;
			}

			return true;
		}

		// Global

		char const *fg_GetSeverityName(ESeverity _Sev)
		{
			switch(_Sev)
			{
				case ESeverity_None:
					return "None";
				case ESeverity_Debug:
					return "Debug";
				case ESeverity_DebugVerbose1:
					return "DebugV1";
				case ESeverity_DebugVerbose2:
					return "DebugV2";
				case ESeverity_Info:
					return "Info";				
				case ESeverity_Warning:
					return "Warning";
				case ESeverity_Error:
					return "Error";
				case ESeverity_Perf_Info:
					return "PInfo";
				case ESeverity_Perf_Warning:
					return "PWarning";
				case ESeverity_Perf_Error:
					return "PError";
				case ESeverity_Critical:
					return "Critical";
				default:
					return "Unknown";
			}
		}

		ESeverity fg_LookupSeverity(CLogStr const& _Name)
		{
			if (_Name.f_CmpNoCase("None") == 0)
				return ESeverity_None;
			else if (_Name.f_CmpNoCase("All") == 0)
				return ESeverity_All;
			else if (_Name.f_CmpNoCase("Debug") == 0)
				return ESeverity_Debug;
			else if (_Name.f_CmpNoCase("DebugV1") == 0)
				return ESeverity_DebugVerbose1;
			else if (_Name.f_CmpNoCase("DebugV2") == 0)
				return ESeverity_DebugVerbose2;
			else if (_Name.f_CmpNoCase("Info") == 0)
				return ESeverity_Info;
			else if (_Name.f_CmpNoCase("Warning") == 0)
				return ESeverity_Warning;
			else if (_Name.f_CmpNoCase("Error") == 0)
				return ESeverity_Error;
			else if (_Name.f_CmpNoCase("Perf_Info") == 0)
				return ESeverity_Perf_Info;
			else if (_Name.f_CmpNoCase("Perf_Warning") == 0)
				return ESeverity_Perf_Warning;
			else if (_Name.f_CmpNoCase("Critical") == 0)
				return ESeverity_Critical;
			else
				return ESeverity_None;
		}

		void fg_LogTo_DebugOut(
				mint _ThreadID
				,	NTime::CTime const& _Time
				,	ESeverity _Sev
				, 	CLogStr const& _Message
				,	NContainer::TCVector<NStr::CStr> const &_Categories
				,	NContainer::TCVector<NStr::CStr> const &_Operations
				,	CLogLocationTag const& _Loc
				)
		{
			NTime::CTimeConvert::CDateTime DateTime;
			NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

			DMibTrace
				(
					"{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}.{fr1,fe3} {sj32} {sj10} {}{\n}"
					, DateTime.m_Year
					<< DateTime.m_Month
					<< DateTime.m_DayOfMonth
					<< DateTime.m_Hour
					<< DateTime.m_Minute
					<< DateTime.m_Second
					<< DateTime.m_Fraction
					<< (_Categories.f_IsEmpty() ? NStr::CStrNonTracked() : NStr::fg_Format<NStr::CStrNonTracked>("<{}>", _Categories.f_GetFirst()))
					<< NStr::fg_Format<NStr::CStrNonTracked>("[{}]", fg_GetSeverityName(_Sev))
					<< _Message
				)
			;
		}
		
		void fg_LogTo_StdErr
			(
				mint _ThreadID
				, NTime::CTime const& _Time
				, ESeverity _Sev
				, CLogStr const& _Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const& _Loc
			)
		{
			NTime::CTimeConvert::CDateTime DateTime;
			NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

			DMibConErrOut
				(
					"{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}.{fr1,fe3} {sj32} {sj10} {}{\n}"
					, DateTime.m_Year
					<< DateTime.m_Month
					<< DateTime.m_DayOfMonth
					<< DateTime.m_Hour
					<< DateTime.m_Minute
					<< DateTime.m_Second
					<< DateTime.m_Fraction
					<< (_Categories.f_IsEmpty() ? NStr::CStrNonTracked() : NStr::fg_Format<NStr::CStrNonTracked>("<{}>", _Categories.f_GetFirst()))
					<< NStr::fg_Format<NStr::CStrNonTracked>("[{}]", fg_GetSeverityName(_Sev))
					<< _Message
				)
			;
		}

		CLogFile::CLogFile()
			: m_bFilenameUsedTime(false)
		{

		}

		CLogFile::~CLogFile()
		{
			if (m_File.f_IsValid())
				m_File.f_Close();
		}

		void CLogFile::f_PrepareFork()
		{
			m_Lock.f_Lock();
			m_Lock.f_PrepareFork();
		}
		
		void CLogFile::f_ForkedChild()
		{
			m_Lock.f_ForkedChild();
			m_Lock.f_Unlock();
		}
		
		void CLogFile::f_ForkedParent()
		{
			m_Lock.f_ForkedParent();
			m_Lock.f_Unlock();
		}

		namespace
		{
			static constexpr EFileOpen gc_LogOpenFlags = EFileOpen_Write | EFileOpen_DontTruncate | EFileOpen_Read | EFileOpen_ShareRead | EFileOpen_NoLocalCache;

			bool fg_RenameLogFile(CLogStr const &_LogFile, CLogStr const &_DestPath, CLogStr const &_Name, CLogStr const &_Extension)
			{
#ifdef DPlatformFamily_Windows
				static constexpr EFileOpen c_CheckOldOpenFlags = gc_LogOpenFlags;
#else
				static constexpr EFileOpen c_CheckOldOpenFlags = EFileOpen_Write | EFileOpen_DontTruncate | EFileOpen_Read | EFileOpen_NoLocalCache;
#endif

				if (!CFile::fs_FileExists(_LogFile, EFileAttrib_File))
					return true;

				NTime::CTime WriteTime;
				{
					// Check if old file is already opened
					CFile TempFile;
					TempFile.f_Open(_LogFile, c_CheckOldOpenFlags);
					WriteTime = TempFile.f_GetWriteTime();
				}

				CLogStr NewLogName = CLogStr::CFormat("{}{}_{tsd_,tsb_,tst_,tss_}.{}") << _DestPath << _Name << WriteTime << _Extension;
				if (CFile::fs_FileExists(NewLogName, EFileAttrib_File))
				{
					int iIndex = 0;
					auto Formatter = CLogStr::CFormat("{}{}_{tsd_,tsb_,tst_,tss_}_{sj2,sf0}.{}");
					Formatter << _DestPath << _Name << WriteTime << iIndex << _Extension;
					for (; iIndex < 100; ++iIndex)
					{
						NewLogName = Formatter;
						if (!CFile::fs_FileExists(NewLogName))
							break;
					}

					if (iIndex >= 100)
						return false;
				}
				CFile::fs_RenameFile(_LogFile, NewLogName);
				return true;
			}

			void fg_RotateLogs(CLogStr const &_Directory, CLogStr const &_Name, CLogStr const &_Extension)
			{
				try
				{
					NTime::CTime OldestAllowed = NTime::CTime::fs_NowUTC() - NTime::CTimeSpanConvert::fs_CreateWeekSpan(1);
					CFile::CFindFilesOptions FindOptions{NStr::fg_Format<CLogStr>("{}{}*.{}", _Directory, _Name, _Extension), false};
					CLogStr ParseRotatedFile = NStr::fg_Format<CLogStr>("{}_{{}_{{}", _Name);
					CLogStr HistoryDirectory = CFile::fs_AppendPath(_Directory, "Older/");

					for (auto &Found : CFile::fs_FindFiles(FindOptions))
					{
						CLogStr FileName = CFile::fs_GetFile(Found.m_Path);

						aint nFound = 0;
						int64 Year = 0;
						int64 Month = 0;

						(CLogStr::CParse(ParseRotatedFile) >> Year >> Month).f_Parse(FileName, nFound);

						bool bIsRotated = nFound > 1;

						try
						{
							if (CFile::fs_GetWriteTime(Found.m_Path) < OldestAllowed)
							{
								if (bIsRotated)
								{
									CLogStr HistoryFileName = HistoryDirectory + FileName;
									if (CFile::fs_FileExists(HistoryFileName))
										fg_RenameLogFile(Found.m_Path, HistoryDirectory, _Name, _Extension);
									else
									{
										CFile::fs_CreateDirectory(HistoryDirectory);
										CFile::fs_RenameFile(Found.m_Path, HistoryFileName);
									}
								}
								else
									fg_RenameLogFile(Found.m_Path, HistoryDirectory, _Name, _Extension);
							}
							else if (!bIsRotated)
								fg_RenameLogFile(Found.m_Path, _Directory, _Name, _Extension);
						}
						catch (CExceptionFile const &)
						{
						}
					}
				}
				catch (CExceptionFile const &)
				{
				}
			}
		}

		bint CLogFile::f_ReadyForWrite()
		{
			if (m_File.f_IsValid())
				return true;

			if (m_Filename.f_FindNoCase("%TIME%") != -1)
			{
				CLogStr TimeStr = NTime::fg_GetFullTimeStr(NTime::CTime::fs_NowLocal());
				TimeStr = TimeStr.f_Replace(":", "_");
				TimeStr = TimeStr.f_Replace("/", "_");
				TimeStr = TimeStr.f_Replace("\\", "_");
				TimeStr = TimeStr.f_Replace(" ", "_");
				TimeStr = TimeStr.f_Replace("-", "_");
				m_Filename = m_Filename.f_Replace("%TIME%", TimeStr);
				m_bFilenameUsedTime = true;
			}

			m_Filename = CFile::fs_GetFile(m_Filename);
			CLogStr DestPath = m_Directory;
			if (DestPath.f_IsEmpty())
				DestPath = CFile::fs_GetLogDirectoryNonTracked();
			
			if (DestPath[DestPath.f_GetLen()-1] != '/')
				DestPath += "/";
			CLogStr Name = CFile::fs_GetFileNoExt(m_Filename);
			CLogStr Extension = CFile::fs_GetExtension(m_Filename);

			if (m_bFilenameUsedTime)
			{
				// Assume time string is unique enough.
				CLogStr LogFile = DestPath + m_Filename;
				try
				{
					CFile::fs_CreateDirectory(DestPath);
					m_File.f_Open(LogFile, EFileOpen_Write | EFileOpen_Read | EFileOpen_ShareRead | EFileOpen_NoLocalCache);
				}
				catch (CExceptionFile const&)
				{
					return false;
				}
				return true;
			}
			else
			{
				auto bOldEnable = NException::fg_SetEnableExceptionTrace(false);
				auto Cleanup
					= fg_OnScopeExit
					(
						[&]
						{
							NException::fg_SetEnableExceptionTrace(bOldEnable);
						}
					)
				;
				
				try
				{
					CLogStr LogFile = DestPath + m_Filename;
					fg_RotateLogs(DestPath, Name, Extension);
					if (!fg_RenameLogFile(LogFile, DestPath, Name, Extension))
						return false;
					CFile::fs_CreateDirectory(DestPath);
					m_File.f_Open(LogFile, gc_LogOpenFlags);
					m_File.f_SetLength(0);
					return true;
				}
				catch (NException::CException const &)
				{
					int iIndex = 0;
					auto Formatter = CLogStr::CFormat("{}{}_{sj2,sf0}.{}");
					Formatter << DestPath << Name << iIndex << Extension;

					for (; iIndex < 100; ++iIndex)
					{
						CLogStr NewName = Formatter;
						try
						{
							if (!fg_RenameLogFile(NewName, DestPath, Name, Extension))
								continue;
							CFile::fs_CreateDirectory(DestPath);
							m_File.f_Open(NewName, gc_LogOpenFlags);
							m_File.f_SetLength(0);
						}
						catch (CExceptionFile const &)
						{					
						}

						if (m_File.f_IsValid())
							return true;
					}
				}

				return false;
			}
		}

		CFileLogger::CFileLogger(CLogFile *_pLogFile)
			: mp_pLogFile(_pLogFile)
		{
		}
			
		void CFileLogger::operator()
			(
				mint _ThreadID
				, NTime::CTime const& _Time
				, ESeverity _Sev
				, CLogStr const& _Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const& _Loc
			)
		{
			CLogFile* pLogFile = mp_pLogFile;
		
			DMibLock(pLogFile->m_Lock);

			if (!pLogFile->f_ReadyForWrite())
				return;

			CFile* pFile = &pLogFile->m_File;

			NTime::CTimeConvert::CDateTime DateTime;
			NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

			CLogStr Text = NStr::fg_Format<CLogStr>
				(
#if 0
					DMibPFileLineFormat " #{nh,sj8,sf0} : "
#endif
					"{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}.{fr1,fe3} {sj32} {sj10} {}{\n}"
#if 0
					, fg_ExtractFileName(_Loc.m_pFile)
					, _Loc.m_Line
					, _ThreadID
#endif
					, DateTime.m_Year
					, DateTime.m_Month
					, DateTime.m_DayOfMonth
					, DateTime.m_Hour
					, DateTime.m_Minute
					, DateTime.m_Second
					, DateTime.m_Fraction
					, (_Categories.f_IsEmpty() ? NStr::CStrNonTracked() : NStr::fg_Format<NStr::CStrNonTracked>("<{}>", _Categories.f_GetFirst()))
					, NStr::fg_Format<NStr::CStrNonTracked>("[{}]", fg_GetSeverityName(_Sev))
					, _Message
				)
			;

			pFile->f_Write(Text.f_GetStr(), Text.f_GetLen() * sizeof(CLogStr::CChar));
			pFile->f_Flush(false); // Optional?
#ifdef DPlatformFamily_OSX
			// Without this no file change notification will be triggered
			pFile->f_SetLength(pFile->f_GetLength());
#endif
		}
#endif


	} // Namespace NLog

} // Namespace NMib
