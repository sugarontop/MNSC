#include "pch.h"
#include "CPrsNode.h"

//do
//
//while(...);

///////////////////////////////////////////////////////////////////////////////////
CPrsDo::CPrsDo(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsDo::CPrsDo(const CPrsDo& src) :CPrsNode(src.m_Symbol)
{
	m_condition = src.m_condition;
	m_statementlist = src.m_statementlist;
}
CPrsDo::~CPrsDo()
{
	Flush();
}

void CPrsDo::Flush()
{	
	m_condition = nullptr;
	m_statementlist = nullptr;
}

void CPrsDo::Generate(stackGntInfo& stinfo)
{
	if (m_condition)
	{
		do
		{
			if (m_statementlist)
			{
				stinfo.top().m_stackInfo.push(CPrsGntInfo::in_loop);

				m_statementlist->Generate(stinfo);

				int brk = stinfo.top().m_stackInfo.top();

				stinfo.top().m_stackInfo.pop();

				if (brk == CPrsGntInfo::outof_loop)
					break;
			}
			m_condition->Generate(stinfo);
		} 
		while (m_condition->enableProcess());
	}
}
void CPrsDo::Parse()
{
	SToken st = getNewSymbol();

	m_statementlist = std::make_shared<CPrsStatmentList>(m_Symbol, ENDSYMBOL_WHILEEND);
	m_statementlist->Parse();

	st = getSymbol();
	if (st.Token == whileSym)
	{
		if (getNewSymbol().Token == lParen)
		{
			getNewSymbol();
			m_condition = std::make_shared<CPrsConditionEx>(m_Symbol);
			m_condition->Parse();
		}
		else
			THROW(L"'(' expected");
	}
	else
		THROW(L"'while' expected");

	if (getSymbol().Token == rParen)
		st = getNewSymbol();

	if (getSymbol().Token == Semicol)
		st = getNewSymbol();

	

}

