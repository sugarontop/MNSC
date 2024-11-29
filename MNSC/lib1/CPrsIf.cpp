#include "pch.h"
#include "CPrsNode.h"

/*
if (...)
	...
	...
elsif (...)
	...
	...
else
	...
	...
endif

*/
///////////////////////////////////////////////////////////////////////////////////
CPrsIf::CPrsIf(CPrsSymbol& sym) :CPrsNode(sym)
{
	//m_statementlist_else = nullptr;
}
CPrsIf::CPrsIf(const CPrsIf& src) :CPrsNode(src.m_Symbol)
{
	//m_statementlist_else = nullptr;

	/*for (UINT i = 0; i < src.m_conAr.size(); i++)
	{
		stThen* pst = new stThen;
		pst->m_condition = nullptr;
		pst->m_statementlist_then = nullptr;

		if (src.m_conAr[i]->m_condition)
			pst->m_condition = new CPrsCondition(*(src.m_conAr[i]->m_condition));
		if (src.m_conAr[i]->m_statementlist_then)
			pst->m_statementlist_then = new CPrsStatmentList(*(src.m_conAr[i]->m_statementlist_then));

		m_conAr.push_back(pst);
	}*/

	m_conAr = src.m_conAr;
	m_statementlist_else = src.m_statementlist_else;
}
CPrsIf::~CPrsIf()
{
	Flush();
}

void CPrsIf::Flush()
{
	/*for (UINT i = 0; i < m_conAr.size(); i++)
	{
		stThen* pst = m_conAr[i];
		delete pst->m_condition;
		delete pst->m_statementlist_then;
		delete pst;
	}*/
	m_conAr.clear();

	m_statementlist_else = nullptr;
}


void CPrsIf::Parse()
{
	SToken st;

	do
	{
		if (getNewSymbol().Token == lParen)
		{
			auto pst = std::make_shared<stThen>();
			//pst->m_condition = NULL;
			//pst->m_statementlist_then = NULL;
			m_conAr.push_back(pst);


			getNewSymbol();
			pst->m_condition = std::make_shared<CPrsConditionEx>(m_Symbol);
			pst->m_condition->Parse();

			st = getSymbol();

			if (st.Token == rParen)
			{
				getNewSymbol();
				pst->m_statementlist_then = std::make_shared <CPrsStatmentList>(m_Symbol, ENDSYMBOL_ENDIF | ENDSYMBOL_ELSE | ENDSYMBOL_ELSIF | ENDSYMBOL_END);
				pst->m_statementlist_then->Parse();
				st = getSymbol();
			}
			else
				THROW(L"')' expected");
		}
		else
			THROW(L"'(' expected");
	} while (st.Token == elsifSym);

	if (st.Token == elseSym)
	{
		getNewSymbol();
		m_statementlist_else = std::make_shared<CPrsStatmentList>(m_Symbol, ENDSYMBOL_ENDIF);
		m_statementlist_else->Parse();
		st = getSymbol();
	}

	if (getSymbol().Token != endifSym)
	{
		THROW(L"'endif' expected");
	}

	getNewSymbol();
}

void CPrsIf::Generate(stackGntInfo& stinfo)
{
	UINT i;
	for (i = 0; i < m_conAr.size(); i++)
	{
		auto pst = m_conAr[i];
		if (pst->m_condition)
		{
			pst->m_condition->Generate(stinfo);
			if (pst->m_condition->enableProcess() && pst->m_statementlist_then)
			{
				pst->m_statementlist_then->Generate(stinfo);
				break;
			}
		}
	}

	if (i == m_conAr.size() && m_statementlist_else)
		m_statementlist_else->Generate(stinfo);
}
