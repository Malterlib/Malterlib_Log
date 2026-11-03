// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

namespace NMib::NLog
{
#if DMibSysLogSeverities
	CSysLogCatScope::CSysLogCatScope(CSystemLogger &_SysLog, char const *_pCategory)
		: m_SysLog(_SysLog)
		, m_pCategory(_pCategory)
	{
		if (*m_pCategory)
			m_SysLog.f_PushCategoryScope(*this);
	}

	CSysLogCatScope::~CSysLogCatScope()
	{
		if (*m_pCategory)
			m_SysLog.f_PopCategoryScope(*this);
	}

	CSysLogOpScope::CSysLogOpScope(CSystemLogger &_SysLog, char const *_pOperation)
		: m_SysLog(_SysLog)
		, m_pOperation(_pOperation)
	{
		if (*m_pOperation)
			m_SysLog.f_PushOperationScope(*this);
	}

	CSysLogOpScope::~CSysLogOpScope()
	{
		if (*m_pOperation)
			m_SysLog.f_PopOperationScope(*this);
	}

	template <typename tf_CMessage, typename ...tfp_CArgs>
	inline_always void fg_SysLog(CLogLocationTag _Loc, ESeverity _Sev, tf_CMessage &&_Msg, tfp_CArgs && ...p_Args)
	{
		NMib::fg_GetSys()->f_GetLogger().f_Log(_Loc, _Sev, NStr::fg_Format<CLogStr>(fg_Forward<tf_CMessage>(_Msg), fg_Forward<tfp_CArgs>(p_Args)...));
	}
#endif
}
