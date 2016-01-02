// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Log_Configuration.h"
#include "Malterlib_Log_Destinations.h"

namespace NMib
{

	namespace NLog
	{
#if DMibSysLogSeverities

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

		struct CLogger::CDetails
		{


			struct CDestination
			{
				FLogDestination* m_pfDestination;
				void* m_pContext;
				CLogFilter m_Filter;
			};

			struct CThreadInfo
			{
				NContainer::TCVector<char const*, NMem::CAllocator_NonTrackedHeap> m_lCategoryStack;
				NContainer::TCVector<char const*, NMem::CAllocator_NonTrackedHeap> m_lOperationStack;
				NContainer::TCVector<CDestination, NMem::CAllocator_NonTrackedHeap> m_lDestinations;
			};

			NThread::TCThreadLocal<CThreadInfo, NMem::CAllocator_NonTrackedHeap> mp_ThreadInfo;

			NThread::CMutual mp_GlobalDestLock; // TODO: Do without? Require global dests set at startup?
			NContainer::TCVector<CDestination, NMem::CAllocator_NonTrackedHeap> mp_lGlobalDestinations;

			NContainer::TCVector< NPtr::TCUniquePointer<CLogFile> > mp_lConfigLogFiles;

		};

		CLogger::CLogger()
		{
			mp_pD = fg_Construct();
		}

		CLogger::~CLogger()
		{
		}

		bint CLogger::f_ReadConfig(CLogStr const& _Path)
		{
			CLogStr Config;

			try
			{
				if (NFile::CFile::fs_FileExists(_Path))
				{
					Config = NFile::CFile::fs_ReadStringFromFile(_Path);
				}
				else
				{
					CLogStr Path = CLogStr::CFormat("{}/{}") << fg_GetSys()->f_GetProgramRootNonTracked() << _Path;
					if (NFile::CFile::fs_FileExists(Path))
						Config = NFile::CFile::fs_ReadStringFromFile(Path);
					else
					{
						CLogStr Path2 = CLogStr::CFormat("{}/{}") << NFile::CFile::fs_GetProgramDirectoryNonTracked() << _Path;
						if (NFile::CFile::fs_FileExists(Path2))
							Config = NFile::CFile::fs_ReadStringFromFile(Path2);
						else
						{
							return false;
						}
					}
				}
			}
			catch(NFile::CExceptionFile const&)
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
				f_PushGlobalDestination(fg_LogTo_DebugOut, nullptr, fg_Move(Filter));
			}
			else if (_Name.f_CmpNoCase("File") == 0)
			{
				if (_lArgs.f_IsEmpty())
					return;

				CLogStr LogFile = _lArgs[0];
				fl_ParseFilter(_lArgs, 1, Filter);

				NPtr::TCUniquePointer<CLogFile> pLogFile = fg_Construct();

				pLogFile->m_Filename = LogFile;

				f_PushGlobalDestination(fg_LogTo_File, pLogFile.f_Get(), fg_Move(Filter));

				mp_pD->mp_lConfigLogFiles.f_Insert(fg_Move(pLogFile));
			}
		}

		void CLogger::f_PushGlobalDestination(FLogDestination* _pFDest, void* _pContext)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			CDetails::CDestination& NewDest = mp_pD->mp_lGlobalDestinations.f_Insert();
			NewDest.m_pfDestination = _pFDest;
			NewDest.m_pContext = _pContext;
		}

		void CLogger::f_RemoveGlobalDestination(FLogDestination* _pFDest)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			aint iDest = 0;
			for (auto &Dest : mp_pD->mp_lGlobalDestinations)
			{
				if (Dest.m_pfDestination == _pFDest)
				{
					mp_pD->mp_lGlobalDestinations.f_Remove(iDest);
					break;
				}
				++iDest;
			}
		}

		void CLogger::f_PushGlobalDestination(FLogDestination* _pFDest, void* _pContext, CLogFilter&& _Filter)
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			CDetails::CDestination& NewDest = mp_pD->mp_lGlobalDestinations.f_Insert();
			NewDest.m_pfDestination = _pFDest;
			NewDest.m_pContext = _pContext;
			NewDest.m_Filter = fg_Move(_Filter);
		}

		void CLogger::f_PopGlobalDestination()
		{
			DMibLock(mp_pD->mp_GlobalDestLock);
			mp_pD->mp_lGlobalDestinations.f_Pop();
		}

		void CLogger::f_PushDestination(FLogDestination* _pFDest, void* _pContext)
		{
			CDetails::CDestination& NewDest = (*mp_pD->mp_ThreadInfo).m_lDestinations.f_Insert();
			NewDest.m_pfDestination = _pFDest;
			NewDest.m_pContext = _pContext;
		}

		void CLogger::f_PushDestination(FLogDestination* _pFDest, void* _pContext, CLogFilter&& _Filter)
		{
			CDetails::CDestination& NewDest = (*mp_pD->mp_ThreadInfo).m_lDestinations.f_Insert();
			NewDest.m_pfDestination = _pFDest;
			NewDest.m_pContext = _pContext;
			NewDest.m_Filter = fg_Move(_Filter);
		}

		void CLogger::f_PopDestination()
		{
			(*mp_pD->mp_ThreadInfo).m_lDestinations.f_Pop();
		}

		void CLogger::f_PushCategoryScope(char const* _Category)
		{
			(*mp_pD->mp_ThreadInfo).m_lCategoryStack.f_Push(_Category);
		}

		void CLogger::f_PopCategoryScope()
		{
			(*mp_pD->mp_ThreadInfo).m_lCategoryStack.f_Pop();
		}

		void CLogger::f_PushOperationScope(char const* _Op)
		{
			(*mp_pD->mp_ThreadInfo).m_lOperationStack.f_Push(_Op);
		}

		void CLogger::f_PopOperationScope()
		{
			(*mp_pD->mp_ThreadInfo).m_lOperationStack.f_Pop();
		}

		void CLogger::f_Log(CLogLocationTag _Loc, ESeverity _Sev, CLogStr const& _Text)
		{
			NTime::CTime LogTime = NTime::CTime::fs_NowLocal();

			mint ThreadID = NSys::fg_Thread_GetCurrentUID();

			auto const& lCats = (*mp_pD->mp_ThreadInfo).m_lCategoryStack;
			auto const& lOps = (*mp_pD->mp_ThreadInfo).m_lOperationStack;

			auto fl_SendToDests =
				[&](NContainer::TCVector<CDetails::CDestination, NMem::CAllocator_NonTrackedHeap>& _lDests)
				{
					for (auto DIter = _lDests.f_GetIterator()
						;DIter
						;++DIter)
					{
						if (	(*DIter).m_Filter.f_Test(
										ThreadID
									,	LogTime
									,	_Sev
									,	_Text
									,	lCats.f_GetArray()
									,	lCats.f_GetLen()
									,	lOps.f_GetArray()
									,	lOps.f_GetLen()
									,	_Loc
								)
							)
						{
							(*(*DIter).m_pfDestination)( 
									(*DIter).m_pContext
								,	ThreadID
								,	LogTime
								,	_Sev
								,	_Text
								,	lCats.f_GetArray()
								,	lCats.f_GetLen()
								,	lOps.f_GetArray()
								,	lOps.f_GetLen()
								,	_Loc
								);
						}
					}

				};


			fl_SendToDests( (*mp_pD->mp_ThreadInfo).m_lDestinations );

			{
				DMibLock(mp_pD->mp_GlobalDestLock);
				fl_SendToDests(mp_pD->mp_lGlobalDestinations);
			}
		}

		// CLogFilter

		bint CLogFilter::f_Test(
				mint _ThreadID
			,	NTime::CTime const& _Time
			,	ESeverity _Sev
			, 	CLogStr const& _Message
			,	char const* const* _pCats
			,	mint _nCats
			,	char const* const* _pOps
			,	mint _nOps
			,	CLogLocationTag const& _Loc
			)
		{
			if (	m_Severity != ESeverity_None
				&&	(_Sev & m_Severity) == 0)
				return false;

			if (	!m_Category.f_IsEmpty())
			{
				mint iC;
				for (iC = 0; iC < _nCats; ++iC)
				{
					if (m_Category.f_CmpNoCase(_pCats[iC]) == 0)
						break;
				}

				if (iC == _nCats)
					return false;
			}

			if (	!m_Operation.f_IsEmpty())
			{
				mint iO;
				for (iO = 0; iO < _nOps; ++iO)
				{
					if (m_Operation.f_CmpNoCase(_pOps[iO]) == 0)
						break;
				}

				if (iO == _nOps)
					return false;
			}

			if (	!m_File.f_IsEmpty())
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

		char const* fg_GetSeverityName(ESeverity _Sev)
		{
			switch(_Sev)
			{
				case ESeverity_None:
					return "None";
				case ESeverity_Debug:
					return "Debug";
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
					void* _pContext // Unused
				,	mint _ThreadID
				,	NTime::CTime const& _Time
				,	ESeverity _Sev
				, 	CLogStr const& _Message
				,	char const* const* _pCats
				,	mint _nCats
				,	char const* const* _pOps
				,	mint _nOps
				,	CLogLocationTag const& _Loc
				)
		{
			NTime::CTimeConvert::CDateTime DateTime;
			NTime::CTimeConvert(_Time).f_ExtractDateTime(DateTime);

			DMibTrace("[{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}] ({sj8}): {sj8}: {}" DMibNewLine
				, 		DateTime.m_Year << DateTime.m_Month << DateTime.m_DayOfMonth
					<<	DateTime.m_Hour << DateTime.m_Minute << DateTime.m_Second
					<< 	fg_GetSeverityName(_Sev)
					<< 	(_nCats ? _pCats[_nCats - 1] : "")
					<<	_Message);
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

			m_Filename = NFile::CFile::fs_GetFile(m_Filename);
			CLogStr DestPath = m_Directory;
			if (DestPath.f_IsEmpty())
				DestPath = NFile::CFile::fs_GetLogDirectoryNonTracked();
			
			if (DestPath[DestPath.f_GetLen()-1] != '/')
				DestPath += "/";
			CLogStr Name = NFile::CFile::fs_GetFileNoExt(m_Filename);
			CLogStr Extension = NFile::CFile::fs_GetExtension(m_Filename);

			if (m_bFilenameUsedTime)
			{
				// Assume time string is unique enough.
				CLogStr LogFile = DestPath + m_Filename;
				try
				{
					NFile::CFile::fs_CreateDirectory(DestPath);
					m_File.f_Open(LogFile, NFile::EFileOpen_Write | NFile::EFileOpen_Read | NFile::EFileOpen_ShareRead);
				}
				catch(NFile::CExceptionFile const&)
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
				
				auto fl_RenameLogFile
					= [&](NStr::CStr const& _LogFile) -> bool
					{
						if (NFile::CFile::fs_FileExists(_LogFile, NFile::EFileAttrib_File))
						{
							// First try to rename the old file
							
							NTime::CTime WriteTime;
							{
								NFile::CFile File;
								File.f_Open(_LogFile, NFile::EFileOpen_ReadAttribs);
								WriteTime = File.f_GetWriteTime();
							}
							CLogStr TimeStr = NTime::fg_GetFullTimeStr(WriteTime);
							TimeStr = TimeStr.f_Replace(":", "_");
							TimeStr = TimeStr.f_Replace("/", "_");
							TimeStr = TimeStr.f_Replace("\\", "_");
							TimeStr = TimeStr.f_Replace(" ", "_");
							TimeStr = TimeStr.f_Replace("-", "_");
							
							CLogStr NewLogName = CLogStr::CFormat("{}{}_{}.{}") << DestPath << Name << TimeStr << Extension;
							if (NFile::CFile::fs_FileExists(NewLogName, NFile::EFileAttrib_File) || true)
							{
								int iIndex = 0;
								auto Formatter = CLogStr::CFormat("{}{}_{}_{}.{}");
								Formatter << DestPath << Name << TimeStr << iIndex << Extension;
								for (; iIndex < 100; ++iIndex)
								{
									NewLogName = Formatter;
									if (!NFile::CFile::fs_FileExists(NewLogName))
										break;
								}
								
								if (iIndex >= 100)
									return false;
							}
							NFile::CFile::fs_RenameFile(_LogFile, NewLogName);
						}
						return true;
					}
				;
				try
				{
					CLogStr LogFile = DestPath + m_Filename;
					if (!fl_RenameLogFile(LogFile))
						return false;
					NFile::CFile::fs_CreateDirectory(DestPath);
					m_File.f_Open(LogFile, NFile::EFileOpen_Write | NFile::EFileOpen_Read | NFile::EFileOpen_ShareRead);
					return true;
				}
				catch (NException::CException const &)
				{
					int iIndex = 0;
					auto Formatter = CLogStr::CFormat("{}{}_{}.{}");
					Formatter << DestPath << Name << iIndex << Extension;

					for (; iIndex < 100; ++iIndex)
					{
						CLogStr NewName = Formatter;
						try
						{							
							if (!fl_RenameLogFile(NewName))
								continue;
							NFile::CFile::fs_CreateDirectory(DestPath);
							m_File.f_Open(NewName, NFile::EFileOpen_Write | NFile::EFileOpen_Read | NFile::EFileOpen_ShareRead);
						}
						catch(NFile::CExceptionFile const&)
						{					
						}

						if (m_File.f_IsValid())
							return true;
					}
				}

				return false;
			}
		}

		void fg_LogTo_File(
					void* _pContext // CLogFile*
				,	mint _ThreadID
				,	NTime::CTime const& _Time
				,	ESeverity _Sev
				, 	CLogStr const& _Message
				,	char const* const* _pCats
				,	mint _nCats
				,	char const* const* _pOps
				,	mint _nOps
				,	CLogLocationTag const& _Loc
				)
		{
			CLogFile* pLogFile = (CLogFile*)_pContext;

			if (!pLogFile->f_ReadyForWrite())
				return;

			NFile::CFile* pFile = &pLogFile->m_File;

			NTime::CTimeConvert::CDateTime DateTime;
			NTime::CTimeConvert(_Time).f_ExtractDateTime(DateTime);

			CLogStr Text = CLogStr::CFormat("{sj32}({sj4}) : #{nh,sj8,sf0} : [{}-{sj2,sf0}-{sj2,sf0} {sj2,sf0}:{sj2,sf0}:{sj2,sf0}] : ({sj8}) : {sj8} : {}" DMibNewLine)
					<<	fg_ExtractFileName(_Loc.m_pFile)
					<<	_Loc.m_Line
					<<	_ThreadID
				 	<<	DateTime.m_Year << DateTime.m_Month << DateTime.m_DayOfMonth
					<<	DateTime.m_Hour << DateTime.m_Minute << DateTime.m_Second
					<< 	fg_GetSeverityName(_Sev)
					<< 	(_nCats ? _pCats[_nCats - 1] : "")
					<<	_Message;

			pFile->f_Write(Text.f_GetStr(), Text.f_GetLen() * sizeof(CLogStr::CChar));

			pFile->f_Flush(false); // Optional?
		}
#endif


	} // Namespace NLog

} // Namespace NMib
