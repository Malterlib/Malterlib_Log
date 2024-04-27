// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Function/Function>
#include <Mib/CommandLine/CommandLine>
#include "Malterlib_Log_Configuration.h"
#include "Malterlib_Log_Destinations.h"
#include "Malterlib_Log.h"
#include "Malterlib_Log_AnsiLogger.h"

namespace NMib::NLog
{
	using namespace NFile;

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
			case ESeverity_DebugVerbose3:
				return "DebugV3";
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
		else if (_Name.f_CmpNoCase("DebugV3") == 0)
			return ESeverity_DebugVerbose3;
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

#if DMibSysLogSeverities
	CSysLogOpScope::CSysLogOpScope(CSysLogOpScope &&_Other) = default;

	void CSysLogOpScope::f_Suspend() noexcept
	{
		m_SysLog.f_PopOperationScope(*this);
	}

	void CSysLogOpScope::f_ResumeNoExcept() noexcept
	{
		m_SysLog.f_PushOperationScope(*this);
	}

	CSysLogCatScope::CSysLogCatScope(CSysLogCatScope &&_Other) = default;

	void CSysLogCatScope::f_Suspend() noexcept
	{
		m_SysLog.f_PopCategoryScope(*this);
	}

	void CSysLogCatScope::f_ResumeNoExcept() noexcept
	{
		m_SysLog.f_PushCategoryScope(*this);
	}

	CNullLogger::CNullLogger()
	{
	}
	CNullLogger::~CNullLogger()
	{
	}

	bool CNullLogger::f_ReadConfig(CLogStr const& _Path)
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

	void CNullLogger::f_PushDestination(FLogDestination &&_fLog, CLogFilter const &_Filter)
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
			bool m_bUseDispatcher = true;
		};

		struct CThreadInfo
		{
			NContainer::TCVector<NStr::CStr> f_GetCategoryScopeStack() const;
			NContainer::TCVector<NStr::CStr> f_GetOperationScopeStack() const;

			DMibListLinkDS_List(CSysLogCatScope, m_Link) m_CategoryStack;
			DMibListLinkDS_List(CSysLogOpScope, m_Link) m_OperationStack;
			NContainer::TCVector<NStorage::TCSharedPointer<CDestination, NMemory::CAllocator_NonTrackedHeap>, NMemory::CAllocator_NonTrackedHeap> m_Destinations;
		};

		NThread::TCThreadLocal<CThreadInfo, NMemory::CAllocator_NonTrackedHeap> mp_ThreadInfo;

		NThread::CMutualManyRead mp_GlobalDestLock; // TODO: Do without? Require global dests set at startup?
		mint mp_NextGlobalDestinationID;
		NContainer::TCMap<mint, NStorage::TCSharedPointer<CDestination, NMemory::CAllocator_NonTrackedHeap>, CSort_Default, NMemory::CAllocator_NonTrackedHeap> mp_GlobalDestinations;

		NContainer::TCVector< NStorage::TCUniquePointer<CLogFile> > mp_lConfigLogFiles;

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

	bool CLogger::f_ReadConfig(CLogStr const& _Path)
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
			f_PushGlobalDestination(fg_LogTo_DebugOut(), fg_Move(Filter));
		}
		else if (_Name.f_CmpNoCase("File") == 0)
		{
			if (_lArgs.f_IsEmpty())
				return;

			CLogStr LogFile = _lArgs[0];
			fl_ParseFilter(_lArgs, 1, Filter);

			NStorage::TCUniquePointer<CLogFile> pLogFile = fg_Construct();

			pLogFile->m_Filename = LogFile;

			f_PushGlobalDestination(CFileLogger(pLogFile.f_Get()), fg_Move(Filter));

			mp_pD->mp_lConfigLogFiles.f_Insert(fg_Move(pLogFile));
		}
	}

	mint CLogger::f_PushGlobalDestination(FLogDestination &&_fLog, bool _bUseDispatcher)
	{
		DMibLock(mp_pD->mp_GlobalDestLock);
		mint ID = ++mp_pD->mp_NextGlobalDestinationID;
		CDetails::CDestination &NewDest = *(mp_pD->mp_GlobalDestinations[ID] = fg_Construct());
		NewDest.m_bUseDispatcher = _bUseDispatcher;
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
		CDetails::CDestination& NewDest = *((*mp_pD->mp_ThreadInfo).m_Destinations.f_Insert() = fg_Construct());
		NewDest.m_fLog = fg_Move(_fLog);
	}

	void CLogger::f_PushDestination(FLogDestination &&_fLog, CLogFilter const &_Filter)
	{
		CDetails::CDestination& NewDest = *((*mp_pD->mp_ThreadInfo).m_Destinations.f_Insert() = fg_Construct());
		NewDest.m_fLog = fg_Move(_fLog);
		NewDest.m_Filter = _Filter;
	}

	void CLogger::f_PopDestination()
	{
		(*mp_pD->mp_ThreadInfo).m_Destinations.f_Pop();
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
						if (Details.mp_Dispatcher && Destination.m_bUseDispatcher)
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
			if (ThreadInfo.m_Destinations.f_IsEmpty() && Details.mp_GlobalDestinations.f_IsEmpty())
				return;

			auto Cats = ThreadInfo.f_GetCategoryScopeStack();
			auto Ops = ThreadInfo.f_GetOperationScopeStack();

			fSendToDests(ThreadInfo.m_Destinations, Cats, Ops);
			fSendToDests(Details.mp_GlobalDestinations, Cats, Ops);
		}
	}

	// CLogFilter

	bool CLogFilter::f_Test
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

	struct CAnsiLoggerConfig
	{
		static ESeverity fs_GetSeveritiesFilter(NStr::CStr const &_SettingsName)
		{
			using namespace NStr;

			ESeverity LogSeverities = ESeverity_All;
			auto CustomSeverities = fg_GetSys()->f_GetEnvironmentVariable("{}Severities"_f << _SettingsName, "");
			if (CustomSeverities)
			{
				LogSeverities = ESeverity_None;
				for (auto &SeverityName : CustomSeverities.f_Split(","))
					LogSeverities |= fg_LookupSeverity(SeverityName.f_Trim());
			}

			return LogSeverities;
		}

		static NCommandLine::EAnsiEncodingFlag fs_GetAnsiFlags(NStr::CStr const &_SettingsName, NCommandLine::EAnsiEncodingFlag _DefaultColor)
		{
			using namespace NStr;

			NCommandLine::EAnsiEncodingFlag Flags = _DefaultColor;
			auto LogSettings = fg_GetSys()->f_GetEnvironmentVariable("{}Color"_f << _SettingsName, {});
			if (LogSettings == "true")
				Flags = NCommandLine::EAnsiEncodingFlag_Color | NCommandLine::EAnsiEncodingFlag_Color24Bit | NCommandLine::EAnsiEncodingFlag_BoxDrawing;
			else if (LogSettings != "false")
			{
				for (auto &Setting : LogSettings.f_Split(","))
				{
					if (Setting == "Color")
						Flags |= NCommandLine::EAnsiEncodingFlag_Color;
					else if (Setting == "Color24Bit")
						Flags |= NCommandLine::EAnsiEncodingFlag_Color24Bit;
					else if (Setting == "ColorLightBackground")
						Flags |= NCommandLine::EAnsiEncodingFlag_ColorLightBackground;
					else if (Setting == "BoxDrawing")
						Flags |= NCommandLine::EAnsiEncodingFlag_BoxDrawing;
				}
			}

			return Flags;
		}

		CAnsiLoggerConfig(NStr::CStr const &_SettingsName, NCommandLine::EAnsiEncodingFlag _DefaultColor, bool _bTrace)
			: m_AnsiLogger(fs_GetAnsiFlags(_SettingsName, _DefaultColor), fs_GetSeveritiesFilter(_SettingsName), _bTrace)
		{
		}

		CLogToStdErrAnsi m_AnsiLogger;
	};

	struct CAnsiLoggerConfig_StdErr : public CAnsiLoggerConfig
	{
		CAnsiLoggerConfig_StdErr()
			: CAnsiLoggerConfig(NStr::gc_Str<"MalterlibStdErrLog">, NCommandLine::CCommandLineDefaults::fs_ColorAnsiFlagsDefault(), false)
		{
		}
	};

	constinit static NStorage::TCAggregate<CAnsiLoggerConfig_StdErr> g_AnsiLogger_StdErr = {DAggregateInit};


	struct CAnsiLoggerConfig_Trace : public CAnsiLoggerConfig
	{
		CAnsiLoggerConfig_Trace()
			: CAnsiLoggerConfig(NStr::gc_Str<"MalterlibTraceLog">, NCommandLine::CCommandLineDefaults::fs_ColorAnsiFlagsDefault(), true)
		{
		}
	};

	constinit static NStorage::TCAggregate<CAnsiLoggerConfig_Trace> g_AnsiLogger_Trace = {DAggregateInit};

	FLogDestination fg_LogTo_DebugOut()
	{
		return [pTraceLogger = &*g_AnsiLogger_Trace]
			(
				mint _ThreadID
				, NTime::CTime const &_Time
				, ESeverity _Sev
				, CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const &_Loc
			)
			{
				pTraceLogger->m_AnsiLogger(_ThreadID, _Time, _Sev, _Message, _Categories, _Operations, _Loc);
			}
		;
	}

	FLogDestination fg_LogTo_StdErr()
	{
		return [pStdErrLogger = &*g_AnsiLogger_StdErr]
			(
				mint _ThreadID
				, NTime::CTime const &_Time
				, ESeverity _Sev
				, CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const &_Loc
			)
			{
				pStdErrLogger->m_AnsiLogger(_ThreadID, _Time, _Sev, _Message, _Categories, _Operations, _Loc);
			}
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
		static constexpr EFileAttrib gc_LogFilePermissions = EFileAttrib_UserRead | EFileAttrib_UserWrite | EFileAttrib_UnixAttributesValid;

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
				TempFile.f_Open(_LogFile, c_CheckOldOpenFlags, gc_LogFilePermissions);
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

	bool CLogFile::f_ReadyForWrite()
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
				m_File.f_Open(LogFile, EFileOpen_Write | EFileOpen_Read | EFileOpen_ShareRead | EFileOpen_NoLocalCache, gc_LogFilePermissions);
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
				m_File.f_Open(LogFile, gc_LogOpenFlags, gc_LogFilePermissions);
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
						m_File.f_Open(NewName, gc_LogOpenFlags, gc_LogFilePermissions);
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

	struct CAnsiLoggerConfig_File : public CAnsiLoggerConfig
	{
		CAnsiLoggerConfig_File()
			: CAnsiLoggerConfig(NStr::gc_Str<"MalterlibFileLog">, NCommandLine::EAnsiEncodingFlag_None, false)
		{
		}
	};

	constinit static NStorage::TCAggregate<CAnsiLoggerConfig_File> g_AnsiLogger_File = {DAggregateInit};

	void CFileLogger::operator()
		(
			mint _ThreadID
			, NTime::CTime const &_Time
			, ESeverity _Sev
			, CLogStr const &_Message
			, NContainer::TCVector<NStr::CStr> const &_Categories
			, NContainer::TCVector<NStr::CStr> const &_Operations
			, CLogLocationTag const &_Loc
		)
	{
		auto &AnsiLogger = *g_AnsiLogger_File;

		CLogFile* pLogFile = mp_pLogFile;

		DMibLock(pLogFile->m_Lock);

		if (!pLogFile->f_ReadyForWrite())
			return;

		CFile* pFile = &pLogFile->m_File;

		NTime::CTimeConvert::CDateTime DateTime;
		NTime::CTimeConvert(_Time.f_ToLocal()).f_ExtractDateTime(DateTime);

		CLogStr Text = AnsiLogger.m_AnsiLogger.f_FormatLog(_ThreadID, _Time, _Sev, _Message, _Categories, _Operations, _Loc);

		pFile->f_Write(Text.f_GetStr(), Text.f_GetLen() * sizeof(CLogStr::CChar));
		pFile->f_Flush(false); // Optional?
#ifdef DPlatformFamily_macOS
		// Without this no file change notification will be triggered
		pFile->f_SetLength(pFile->f_GetLength());
#endif
	}
#endif
}
