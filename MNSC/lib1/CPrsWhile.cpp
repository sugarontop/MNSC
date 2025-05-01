#include "pch.h"
#include "CPrsNode.h"

/*
while ( ... )

	...
end


*/
///////////////////////////////////////////////////////////////////////////////////
CPrsWhile::CPrsWhile(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsWhile::CPrsWhile(const CPrsWhile& src) :CPrsNode(src.m_Symbol)
{
	m_condition = src.m_condition;
	m_statementlist = src.m_statementlist;
}
CPrsWhile::~CPrsWhile()
{
	Flush();
}

void CPrsWhile::Flush()
{
	m_condition = nullptr;
	m_statementlist = nullptr;
}

void CPrsWhile::Generate(stackGntInfo& stinfo)
{
	if (m_condition)
	{
		m_condition->Generate(stinfo);
		while (m_condition->enableProcess())
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
	}
}
void CPrsWhile::Parse()
{
	SToken st = getNewSymbol();

	if (st.Token == lParen)
	{
		getNewSymbol();
		m_condition = std::make_shared<CPrsConditionEx>(m_Symbol);
		m_condition->Parse();
	}
	else
		THROW(L"'(' expected");

	if (getSymbol().Token == rParen)
	{
		getNewSymbol();
		m_statementlist = std::make_shared <CPrsStatmentList>(m_Symbol, ENDSYMBOL_END);
		m_statementlist->Parse();

		if (getSymbol().Token != endSym)
		{
			THROW(L"'end' expected");
		}

		getNewSymbol();
	}
	else
		THROW(L"')' expected");
}

