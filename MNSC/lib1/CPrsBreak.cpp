#include "pch.h"
#include "CPrsNode.h"

CPrsBreak::CPrsBreak(CPrsSymbol& sym) :CPrsNode(sym)
{
}
CPrsBreak::CPrsBreak(const CPrsBreak& src) :CPrsNode(src.m_Symbol)
{
}
CPrsBreak::~CPrsBreak()
{
	Flush();
}
void CPrsBreak::Parse()
{
	DEBUG_PARSE(CPrsBreak)
	if (getSymbol().Token == breakSym)
	{
		if (getNewSymbol().Token != Semicol)
		{
			THROW(L"';' expected");
		}
		getNewSymbol();
	}
}
void CPrsBreak::Generate(stackGntInfo& stinfo)
{
	if (stinfo.top().m_stackInfo.top() == CPrsGntInfo::in_loop)
		stinfo.top().m_stackInfo.top() = CPrsGntInfo::outof_loop;
}

