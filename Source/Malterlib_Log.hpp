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
#endif
}
