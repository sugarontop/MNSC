#include "pch.h"
#include "CPrsNode.h"

///////////////////////////////////////////////////////////////////////////////////
CPrsReturn::CPrsReturn(CPrsSymbol& sym) :CPrsNode(sym)
{
	
}
CPrsReturn::CPrsReturn(const CPrsReturn& src) :CPrsNode(src.m_Symbol)
{	
	m_expression = src.m_expression;
}
CPrsReturn::~CPrsReturn()
{
	Flush();
}
void CPrsReturn::Parse()
{
	SToken st = getSymbol();
	if (st.Token == returnSym)
	{
		st = getNewSymbol();

		m_expression = std::make_shared <CPrsExpression>(m_Symbol);
		m_expression->Parse();

		if (getSymbol().Token != Semicol)
		{
			THROW(L"';' expected");
		}
		getNewSymbol();
	}
}
void CPrsReturn::Generate(stackGntInfo& stinfo)
{
	m_expression->Generate(stinfo);

	// stinfo‚Ìtop‚Ìî•ñ‚ð—^‚¦‚é
	// •Ô‚è’l‚Æ‚±‚êˆÈã‚Ì‚±‚ÌŠK‘w‚ÌGenerate‚ð’†Ž~‚·‚é

	//FVariant v = m_expression->getValue();

	stinfo.top().m_ReturnValue = m_expression->getValue();
	stinfo.top().m_bReturn = true;
}
void CPrsReturn::Flush()
{
	m_expression = nullptr;
}

