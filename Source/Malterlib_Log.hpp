// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib::NLog
{
#if DMibSysLogSeverities
	CSysLogCatScope::CSysLogCatScope(CSystemLogger &_SysLog, char const *_pCategory)
		: m_SysLog(_SysLog)
		, m_pCategory(_pCategory)
	{
		DMibThreadLocalScopeEnter;
		m_SysLog.f_PushCategoryScope(*this);
	}

	CSysLogCatScope::~CSysLogCatScope()
	{
		m_SysLog.f_PopCategoryScope(*this);
		DMibThreadLocalScopeExit;
	}

	CSysLogOpScope::CSysLogOpScope(CSystemLogger &_SysLog, char const *_pOperation)
		: m_SysLog(_SysLog)
		, m_pOperation(_pOperation)
	{
		DMibThreadLocalScopeEnter;
		m_SysLog.f_PushOperationScope(*this);
	}

	CSysLogOpScope::~CSysLogOpScope()
	{
		m_SysLog.f_PopOperationScope(*this);
		DMibThreadLocalScopeExit;
	}
#endif
}
