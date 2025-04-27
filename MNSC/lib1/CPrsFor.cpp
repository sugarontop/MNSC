#include "pch.h"
#include "CPrsNode.h"
#include "mnsc.h"
///////////////////////////////////////////////////////////////////////////////////
CPrsFor::CPrsFor(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsFor::CPrsFor(const CPrsFor& src) :CPrsNode(src.m_Symbol)
{
	m_condition = src.m_condition;
	m_statementlist = src.m_statementlist;
	m_ini_assign = src.m_ini_assign;
	m_icr_assign = src.m_icr_assign;
	in_parameters_ = src.in_parameters_;
	in_ident_nm_ = src.in_ident_nm_;
	in_object_nm_ = src.in_object_nm_;	
	m_in_expression = src.m_in_expression;
}
CPrsFor::~CPrsFor()
{
	Flush();
}

void CPrsFor::Flush()
{
	m_condition = nullptr;
	m_statementlist = nullptr;
	m_ini_assign = nullptr;
	m_icr_assign = nullptr;
	in_parameters_ = nullptr;
	
	m_in_expression = nullptr;
	in_ident_nm_.clear();
	in_object_nm_.clear();
}
void CPrsFor::Parse2()
{
	SToken st = getSymbol();
	if ( st.Token == Ident )
	{
		in_ident_nm_ = st.Value;
		st = getNewSymbol();
		if ( st.Token == inSym )
		{
			st = getNewSymbol();
			if ( st.Token == Ident && st.Value == L"range" )
			{
				st = getNewSymbol();
				if ( st.Token == lParen )
				{
					in_parameters_ = std::make_shared<CPrsParameters>(m_Symbol);
					in_parameters_->Parse();
				}
			}
			
			else if (st.Token == Ident)
			{
				in_object_nm_ = st.Value;

				
				auto olds = m_Symbol.setStat(CPrsSymbol::STAT::IN_PARSING);
				m_in_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_in_expression->Parse();
				m_Symbol.setStat(olds);
				st = getSymbol();
		
			}
			else if (st.Token == lSquare)
			{
				in_object_nm_ = L"square";
				auto olds = m_Symbol.setStat(CPrsSymbol::STAT::IN_PARSING);
				m_in_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_in_expression->Parse();
				m_Symbol.setStat(olds);
				st = getSymbol();


			}

		}
	}
}

void CPrsFor::Parse()
{
	SToken st = getNewSymbol();

	if (st.Token == lParen)
	{
		getNewSymbol();
		m_ini_assign = std::make_shared<CPrsAssign>(m_Symbol);
		m_ini_assign->Parse();

		m_condition = std::make_shared <CPrsConditionEx>(m_Symbol);
		m_condition->Parse();

		st = getNewSymbol();

		m_icr_assign = std::make_shared <CPrsAssignPlusEqual>(m_Symbol);
		m_icr_assign->Parse();
	}
	else if (st.Token == Ident )
	{
		Parse2();
		goto jump1;
	}
	else
		THROW(L"'(' expected");

	if (getSymbol().Token == rParen)
	{
jump1:
		getNewSymbol();
		m_statementlist = std::make_shared <CPrsStatmentList>(m_Symbol, ENDSYMBOL_ENDNEXT);
		m_statementlist->Parse();
		st = getSymbol();

		if (getSymbol().Token != nextSym)
		{
			THROW(L"'next' expected");
		}

		getNewSymbol();

	}
	else
		THROW(L"')' expected");
}

void CPrsFor::Generate(stackGntInfo& stinfo)
{
	if (m_statementlist == nullptr)
		return;


	if (m_condition)
	{
		m_ini_assign->Generate(stinfo);
		m_condition->Generate(stinfo);
		while (m_condition->enableProcess())
		{
			
			{				
				stinfo.top().m_stackInfo.push(CPrsGntInfo::in_loop);

				m_statementlist->Generate(stinfo);

				int brk = stinfo.top().m_stackInfo.top();

				stinfo.top().m_stackInfo.pop();

				if (brk == CPrsGntInfo::outof_loop)
					break;
			}

			m_icr_assign->Generate(stinfo);
			m_condition->Generate(stinfo);
		}
	}
	else if (in_parameters_)
	{
		in_parameters_->Generate(stinfo);
		auto prms = in_parameters_->getParam();
		__int64 s=0,e=0,step=1;
		if (prms.size() >= 2)
		{
			s = prms[0].getN();
			e = prms[1].getN();

			if (prms.size() >= 3)
				step = prms[2].getN();
		}
		else if (prms.size() == 1)
		{
			e = prms[0].getN();
		}
	
		for(auto i = s; i < e; i +=step)
		{
			FVariant f(i);
			m_Symbol.getSymbolTable().setAt(in_ident_nm_, f);
						
			{
				stinfo.top().m_stackInfo.push(CPrsGntInfo::in_loop);

				m_statementlist->Generate(stinfo);

				int brk = stinfo.top().m_stackInfo.top();

				stinfo.top().m_stackInfo.pop();

				if (brk == CPrsGntInfo::outof_loop)
					break;
			}			
		}
	}
	else if (!in_object_nm_.empty())
	{
		//FVariant ar;
		//m_Symbol.getSymbolTable().getAt(in_object_nm_, ar);


		m_in_expression->Generate(stinfo);
		const FVariant& ar = m_in_expression->getValue();


		if (ar.vt == VT_UNKNOWN)
		{
			auto par = dynamic_cast<IVARIANTArray*>(ar.punkVal);
			if (par == nullptr)
				THROW(L"for in object err");

			auto cnt = par->Count();

			for (ULONG i = 0; i < cnt; i++)
			{
				VARIANT v;
				::VariantInit(&v);

				par->Get(i, &v);

				FVariant fv(v);
				m_Symbol.getSymbolTable().setAt(in_ident_nm_, fv);
				//::VariantClear(&v);

				{
					stinfo.top().m_stackInfo.push(CPrsGntInfo::in_loop);

					m_statementlist->Generate(stinfo);

					int brk = stinfo.top().m_stackInfo.top();

					stinfo.top().m_stackInfo.pop();

					if (brk == CPrsGntInfo::outof_loop)
						break;
				}
			}
		}
	}
	
	else
		THROW(L"CPrsFor err");
}