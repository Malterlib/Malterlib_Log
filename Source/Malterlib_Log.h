// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

/*

Malterlib Logging System

Logs messages, optionally tagged with a Category and Operation, to various configurable, filterable, destinations.

Hopefully not TL;DR:

	Reading the default log file format:

		File(Line)					    ThreadID	   Date & Time			Severity	Category	Message
		----------					    --------	   -----------			--------	--------	-------

		aoqtexperimental_main.cpp(79) : #0000189c : [2012-10-09 12:59:51] : (    Info) :          : AOQTExperimental
		aoqtexperimental_main.cpp(83) : #0000189c : [2012-10-09 12:59:51] : ( Warning) :   "Flux" : Warning!
		aoqtexperimental_main.cpp(90) : #0000189c : [2012-10-09 12:59:51] : (   Error) :          : Error!
		
		Note that categories are actually a stack. A log message will be tagged to all categories it is tagged with,
		but the default log files only show the latest.

		Operation (see below) is also a stack but it not shown by default. It is more useful for filtering.
		
	Log with DLog:

		DLog(<Severity>, "<Message>", Args...)

		Where <Severity> is one of:
			Debug			// Spam Spam Spam
			Info			// Hey, check this out
			Warning			// Something failed, but we can fight on
			Error			// Bad situation
			Perf_Info		// Performance Info: This took this long
			Perf_Warning	// Performance Warning: Profile me?
			Perf_Error		// Performance Error: Too Damn Slow
			Critical		// OMG

		e.g.

			DLog(Warning, "{} is larger than 10", X);

	Scoped categories with DLogCategory:

		{
			DLogCategory(My Category);

			// Anything logged here will be tagged to My Category
		}

	Scoped operations with DLogOperation:

		{
			DLogOperation(My Operation);

			// Anything logged here will be tagged to My Operation
		}

	Different sets of severities are logged with different build configs:

		Config			Logged Severities
		------			-----------------
		Release			Nothing
		
		Debug			Debug, Info, Warning, Error, Critical

		Profile			Error, Critical, Perf_Info, Perf_Warning, Perf_Error

		The Rest		Warning, Error, Critical
		

		(See Malterlib_Log_Configuration.h for setting these)

		When a config disabled a severity any macros of the form
			DLog(<Severity>, ...) 
		become NOPs (nothing).

		When all severities are disabled all logging macros become nops. 

	Log Destinations:
		
		By default log messages just go to Debug Out.

		You can specify a config file in the dir with your exe named:
			"Malterlib_Log_Config.txt"
		to specify global log destinations and their filters.

		(Note: The config file will be converted into a registry file in the future)

		Log config format:
		
			<Dest>: <Arg>, <Arg>, ...

		Supported destinations:

			DebugOut: <Severities>, <Category>, <Operation>, <File>

			File: <Filename>, <Severities>, <Category>, <Operation>, <File>

			You may leave any unneeded filter arguments empty.

			The <File> filter argument is treated as a suffix. e.g.
				main.cpp
					matches
				MyApp\main.cpp

		Example:

			DebugOut: All
			File: Log.txt, All
			File: Log_Errors.txt, Error|Critical
			File: Log_My.txt, All, MikesCategory
			File: Log_MyFile.txt, All, , , MyFile.cpp

		You can also log all messages on one thread within a given scope to a log file from code:

			{
				DLogToFile("<FileName>");

				// Anything logged here goes to <Filename> as well as the defaults.
			}

				or

			{
				DLogToFile(
						"<FileName>"
					,	NLog::CLogFilter(
								NLog::ESeverity_Debug		// Severity bitfield.
						// The following can be left out if not required:
							,	NLog::CLogStr()				// Category
							,	NLog::CLogStr()				// Operation
							,	NLog::CLogStr()				// File
						)
				);

				// Anything logged here with severity Debug goes to <Filename> as well as the defaults.
			}

*/
#pragma once
#include <Mib/Core/Core>
#include "Malterlib_Log_Configuration.h"

#ifndef DMibSysLogSeverities
	#define DMibSysLogSeverities DMibLogSeverity_All
#endif

namespace NMib
{
	static inline_small CSystem *fg_GetSys();
}

namespace NMib::NLog
{

	enum ESeverity
	{
		ESeverity_None		= 0 // Only to be used in filters

		, ESeverity_Debug			= DMibBit(0)
		, ESeverity_DebugVerbose1	= DMibBit(1)
		, ESeverity_DebugVerbose2	= DMibBit(2)
		, ESeverity_Info			= DMibBit(3)
		, ESeverity_Warning			= DMibBit(4)
		, ESeverity_Error			= DMibBit(5)

		, ESeverity_Perf_Info		= DMibBit(6)
		, ESeverity_Perf_Warning	= DMibBit(7)
		, ESeverity_Perf_Error		= DMibBit(8)

		, ESeverity_Critical		= DMibBit(9)

		, ESeverity_DebugVerbose3	= DMibBit(10)

		, ESeverity_All				= DMibBit(11) - 1	 // Only to be used in filters
	};

	using CLogStr = NStr::CStrNonTracked;

	struct CLogLocationTag
	{
		CLogLocationTag() = default;

		CLogLocationTag(ch8 const *_pFile, int _Line)
			: m_pFile(_pFile)
			, m_Line(_Line)
		{
		}

		CLogLocationTag(CLogLocationTag const& _ToCopy)
			: m_pFile(_ToCopy.m_pFile)
			, m_Line(_ToCopy.m_Line)
		{
		}

		ch8 const *m_pFile = nullptr;
		int m_Line = 0;
	};

	char const* fg_GetSeverityName(ESeverity _Sev);
	ESeverity fg_LookupSeverity(CLogStr const& _Name);

#if DMibSysLogSeverities
	class CLogger;
	class CNullLogger;

	#if (DMibSysLogSeverities) != 0
		using CSystemLogger = CLogger;
	#else
		using CSystemLogger = CNullLogger;
	#endif

	struct CSysLogCatScope : public CCoroutineThreadLocalHandler
	{
		inline CSysLogCatScope(CSystemLogger &_SysLog, char const *_pCategory);
		inline ~CSysLogCatScope();
		CSysLogCatScope(CSysLogCatScope &&_Other);
		void f_Suspend() noexcept override;
		void f_ResumeNoExcept() noexcept override;

		CSystemLogger &m_SysLog;
		ch8 const *m_pCategory;
		DMibListLinkDS_Link(CSysLogCatScope, m_Link);
	};

	struct CSysLogOpScope : public CCoroutineThreadLocalHandler
	{
		inline CSysLogOpScope(CSystemLogger &_SysLog, char const *_pOperation);
		inline ~CSysLogOpScope();
		CSysLogOpScope(CSysLogOpScope &&_Other);
		void f_Suspend() noexcept override;
		void f_ResumeNoExcept() noexcept override;

		CSystemLogger &m_SysLog;
		ch8 const *m_pOperation;
		DMibListLinkDS_Link(CSysLogOpScope, m_Link);
	};

	using FLogDestination = NFunction::TCFunctionMovable
		<
			void
			(
				mint _ThreadID
				, NTime::CTime const& _Time
				, ESeverity _Sev
				, CLogStr const& _Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const& _Loc
			)
		>
	;

	using FLogDispatch = NFunction::TCFunctionMovable
		<
			void (NFunction::TCFunctionMovable<void ()> &&_fToDispatch)
		>
	;

	struct CLogFilter
	{
		CLogFilter() = default;

		CLogFilter
			(
				ESeverity _FilterOnSeverity
				, CLogStr const &_FilterOnCategory = {}
				, CLogStr const &_FilterOnOperation = {}
				, CLogStr const &_FilterOnFile = {}
			)
			: m_Severity(_FilterOnSeverity)
			, m_Category(_FilterOnCategory)
			, m_Operation(_FilterOnOperation)
			, m_File(_FilterOnFile)
		{
		}

		CLogFilter(CLogFilter const &) = default;
		CLogFilter(CLogFilter &&) = default;
		CLogFilter &operator=(CLogFilter &&) = default;
		CLogFilter &operator=(CLogFilter const &) = default;

		bool f_Test
			(
				mint _ThreadID
				, NTime::CTime const &_Time
				, ESeverity _Sev
				, CLogStr const &_Message
				, NContainer::TCVector<NStr::CStr> const &_Categories
				, NContainer::TCVector<NStr::CStr> const &_Operations
				, CLogLocationTag const &_Loc
			)
		;

		ESeverity m_Severity = ESeverity_None;
		CLogStr m_Category;
		CLogStr m_Operation;
		CLogStr m_File;
	};

	#define DLogLocTag NMib::NLog::CLogLocationTag(__FILE__, __LINE__)

	class CLogger
	{
	protected:

		enum
		{
			Max_Scope_Depth = 32,
			Max_Dest_Depth = 8,
		};

		struct CDetails;
		NStorage::TCUniquePointer<CDetails> mp_pD;

		void fp_AddGlobalDestination(CLogStr const& _Name, NContainer::TCVector<CLogStr> const& _lArgs);

	public:

		CLogger();
		~CLogger();

		bool f_ReadConfig(CLogStr const& _Path);

		mint f_PushGlobalDestination(FLogDestination &&_fLog, bool _bUseDispatcher = true);
		mint f_PushGlobalDestination(FLogDestination &&_fLog, CLogFilter&& _Filter);
		bool f_PopGlobalDestination();
		void f_RemoveGlobalDestination(mint _ID);

		void f_PushDestination(FLogDestination &&_fLog);
		void f_PushDestination(FLogDestination &&_fLog, CLogFilter const &_Filter);
		void f_PopDestination();

		void f_PushCategoryScope(CSysLogCatScope &_Scope);
		void f_PopCategoryScope(CSysLogCatScope &_Scope);

		void f_PushOperationScope(CSysLogOpScope &_Scope);
		void f_PopOperationScope(CSysLogOpScope &_Scope);

		void f_Log(CLogLocationTag _Loc, ESeverity _Sev, CLogStr const& _Str);

		void f_SetDispatcher(FLogDispatch &&_fDispatcher);

		// Internal
		void f_Submit(ESeverity _Sev, CLogStr&& _Text);

		// Disable
		void f_PushCategoryScope(NStr::CStr const& _Str);
		void f_PushCategoryScope(NStr::CWStr const& _Str);
		void f_PushCategoryScope(NStr::CUStr const& _Str);

		void f_PushOperationScope(NStr::CStr const& _Str);
		void f_PushOperationScope(NStr::CWStr const& _Str);
		void f_PushOperationScope(NStr::CUStr const& _Str);

		void f_PrepareFork();
		void f_ForkedChild();
		void f_ForkedParent();

	};

	class CNullLogger
	{

	public:
		CNullLogger();
		~CNullLogger();

		bool f_ReadConfig(CLogStr const& _Path);

		mint f_PushGlobalDestination(FLogDestination &&_fLog);
		mint f_PushGlobalDestination(FLogDestination &&_fLog, CLogFilter&& _Filter);
		bool f_PopGlobalDestination();
		void f_RemoveGlobalDestination(mint _ID);

		void f_PushDestination(FLogDestination &&_fLog);
		void f_PushDestination(FLogDestination &&_fLog, CLogFilter const &_Filter);
		void f_PopDestination();

		void f_PushCategoryScope(CSysLogCatScope &_Scope);
		void f_PopCategoryScope(CSysLogCatScope &_Scope);

		void f_PushOperationScope(CSysLogOpScope &_Scope);
		void f_PopOperationScope(CSysLogOpScope &_Scope);

		void f_Log(CLogLocationTag _Loc, ESeverity _Sev, CLogStr const& _Str);

		// Internal
		void f_Submit(ESeverity _Sev, CLogStr&& _Text);

		// Disable
		void f_PushCategoryScope(NStr::CStr const& _Str);
		void f_PushCategoryScope(NStr::CWStr const& _Str);
		void f_PushCategoryScope(NStr::CUStr const& _Str);

		void f_PushOperationScope(NStr::CStr const& _Str);
		void f_PushOperationScope(NStr::CWStr const& _Str);
		void f_PushOperationScope(NStr::CUStr const& _Str);
	};

	template<typename tf_CMessage, typename... tfp_CArgs>
	inline_always void fg_SysLog(CLogLocationTag _Loc, ESeverity _Sev, tf_CMessage &&_Msg, tfp_CArgs &&...p_Args)
	{
		NMib::fg_GetSys()->f_GetLogger().f_Log(_Loc, _Sev, NStr::fg_Format<CLogStr>(fg_Forward<tf_CMessage>(_Msg), fg_Forward<tfp_CArgs>(p_Args)...));
	}

#endif
// Internal Macros:
	#define DMibLog_SevPaster(_Sev) DMibLog_##_Sev
	#define DMibLogArgHelper(_R, _Data, _Elem)	<< (_Elem)
	#define DMibLogWrap(...) (__VA_ARGS__)

	#if (DMibSysLogSeverities) & DMibLogSeverity_Debug
		#define DMibLog_Debug(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Debug, __VA_ARGS__)
	#else
		#define DMibLog_Debug(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_DebugVerbose1
		#define DMibLog_DebugVerbose1(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_DebugVerbose1, __VA_ARGS__)
	#else
		#define DMibLog_DebugVerbose1(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_DebugVerbose2
		#define DMibLog_DebugVerbose2(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_DebugVerbose2, __VA_ARGS__)
	#else
		#define DMibLog_DebugVerbose2(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_DebugVerbose3
		#define DMibLog_DebugVerbose3(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_DebugVerbose3, __VA_ARGS__)
	#else
		#define DMibLog_DebugVerbose3(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Info
		#define DMibLog_Info(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Info, __VA_ARGS__)
	#else
		#define DMibLog_Info(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Warning
		#define DMibLog_Warning(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Warning, __VA_ARGS__)
	#else
		#define DMibLog_Warning(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Error
		#define DMibLog_Error(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Error, __VA_ARGS__)
	#else
		#define DMibLog_Error(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Perf_Info
		#define DMibLog_Perf_Info(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Perf_Info, __VA_ARGS__)
	#else
		#define DMibLog_Perf_Info(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Perf_Warning
		#define DMibLog_Perf_Warning(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Perf_Warning, __VA_ARGS__)
	#else
		#define DMibLog_Perf_Warning(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Perf_Error
		#define DMibLog_Perf_Error(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Perf_Error, __VA_ARGS__)
	#else
		#define DMibLog_Perf_Error(...) (void)0
	#endif

	#if (DMibSysLogSeverities) & DMibLogSeverity_Critical
		#define DMibLog_Critical(...) NMib::NLog::fg_SysLog(DLogLocTag, NMib::NLog::ESeverity_Critical, __VA_ARGS__)
	#else
		#define DMibLog_Critical(...) (void)0
	#endif

// Public Macros:

	#define DMibLog(_Sev, ...) DMibLog_SevPaster(_Sev)(__VA_ARGS__)

	#if (DMibSysLogSeverities) != 0
		#define DMibLogCategory(_Category) NMib::NLog::CSysLogCatScope l_Cat##__LINE__(NMib::fg_GetSys()->f_GetLogger(), #_Category)
		#define DMibLogCategoryEx(_Tag, _Category) NMib::NLog::CSysLogCatScope l_Cat##__LINE__##_Tag(NMib::fg_GetSys()->f_GetLogger(), #_Category)
		#define DMibLogCategoryStr(d_Category) NMib::NLog::CSysLogCatScope l_Cat##__LINE__(NMib::fg_GetSys()->f_GetLogger(), d_Category)

		#define DMibLogOperation(_Op) NMib::NLog::CSysLogOpScope l_Cat##__LINE__(NMib::fg_GetSys()->f_GetLogger(), #_Op)
		#define DMibLogOperationEx(_Tag, _Op) NMib::NLog::CSysLogOpScope l_Cat##__LINE__##_Tag(NMib::fg_GetSys()->f_GetLogger(), #_Op)
	#else
		#define DMibLogCategory(_Category) (void)0
		#define DMibLogCategoryEx(_Tag, _Category) (void)0
		#define DMibLogCategoryStr(d_Category) (void)0

		#define DMibLogOperation(_Op) (void)0
		#define DMibLogOperationEx(_Tag, _Op) (void)0
	#endif

	#define DMibLogCat(_Category) DMibLogCategory(_Category)
	#define DMibLogCatEx(_Tag, _Category) DMibLogCategoryEx(_Tag, _Category)
	#define DMibLogOp(_Op) DMibLogOperation(_Op)
	#define DMibLogOpEx(_Tag, _Op) DMibLogOperationEx(_Tag, _Op)

	#define DMibLogWithCategory(d_Category, d_Severity, ...) [&]{DMibLogCategory(d_Category); DMibLog(d_Severity, __VA_ARGS__);}()
	#define DMibLogWithCategoryStr(d_Category, d_Severity, ...) [&](::NMib::NStr::CStr const &_Category){DMibLogCategoryStr(_Category.f_GetStr()); DMibLog(d_Severity, __VA_ARGS__);}(d_Category)

	#ifndef DMibPNoShortCuts
		#define DLog(_Sev, ...) DMibLog(_Sev, __VA_ARGS__)
		#define DLogWithCategory DMibLogWithCategory
		#define DLogWithCategoryStr DMibLogWithCategoryStr
		#define DLogCategoryStr DMibLogCategoryStr

		#define DLogCategory(_Category) DMibLogCategory(_Category)
		#define DLogCategoryEx(_Tag, _Category) DMibLogCategoryEx(_Tag, _Category)
		#define DLogOperation(_Op) DMibLogOperation(_Op)
		#define DLogOperationEx(_Tag, _Op) DMibLogOperationEx(_Tag, _Op)

		#define DLogCat(_Category) DMibLogCat(_Category)
		#define DLogCatEx(_Tag, _Category) DMibLogCatEx(_Tag, _Category)
		#define DLogOp(_Op) DMibLogOp(_Op)
		#define DLogOpEx(_Tag, _Op) DMibLogOpEx(_Tag, _Op)
	#endif

}

#include "Malterlib_Log.hpp"

#ifndef DMibPNoShortCuts
	using namespace NMib::NLog;
#endif
