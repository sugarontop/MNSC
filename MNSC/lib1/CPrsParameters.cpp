#include "pch.h"
#include "CPrsNode.h"


///////////////////////////////////////////////////////////////////////////////////

CPrsParameters::CPrsParameters(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsParameters::CPrsParameters(const CPrsParameters& src) :CPrsNode(src.m_Symbol), m_lsVal(src.m_lsVal)
{
	m_lsExp = src.m_lsExp;
}

CPrsParameters::~CPrsParameters()
{
	Flush();
}

void CPrsParameters::Flush()
{
	m_lsExp.clear();
}
std::vector<FVariant>& CPrsParameters::getParam()
{
	return m_lsVal;
}
void CPrsParameters::Generate(stackGntInfo& stinfo)
{
	m_lsVal.clear();
	
	for(auto& expression : m_lsExp)
	{		
		expression->Generate(stinfo);
		const FVariant& var = expression->getValue();

		// varをストックしておく
		m_lsVal.push_back(var);
	}
}

void CPrsParameters::Parse()
{
	SToken	st = getSymbol();

	if (st.Token != lParen) return;

	do
	{
		st = getNewSymbol();
		if (st.Token == Ident || st.Token == Number || st.Token == Minus || st.Token == Quotation || 
			st.Token == Float || st.Token == lSquare || st.Token == lBracket ||
			st.Token == trueSym || st.Token == falseSym)
		{
			auto expression = std::make_shared<CPrsExpression>(m_Symbol);
			expression->Parse();
			m_lsExp.push_back(expression);

			st = getSymbol();
		}
	} while (st.Token == Comma);

	if (st.Token != rParen )
		if ( st.Token != Semicol)
			THROW(L"')' expected");
}
void CPrsParameters::ParseBracket()
{
	SToken		st = getSymbol();

	if (st.Token != lSquare) return;	// token=='['

	st = getNewSymbol();
	if (st.Token == Ident || st.Token == Number)
	{
		auto expression = std::make_shared<CPrsExpression>(m_Symbol);
		expression->Parse();
		m_lsExp.push_back(expression);

		st = getSymbol();
	}

	if (st.Token != rSquare)
		THROW(L"']' expected");
}
void CPrsParameters::ParseParen2()
{
	SToken		st = getSymbol();

	if (st.Token != lBracket) return;	// token=='{'

	st = getNewSymbol();
	if (st.Token == Ident || st.Token == Quotation)
	{
		auto expression = std::make_shared<CPrsExpression>(m_Symbol);
		expression->Parse();
		m_lsExp.push_back(expression);

		st = getSymbol();
	}

	if (st.Token != rBracket)
		THROW(L"'}' expected");
}


