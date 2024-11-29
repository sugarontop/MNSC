#include "pch.h"
#include "CPrsNode.h"

///////////////////////////////////////////////////////////////////////////////////
CPrsPrint::CPrsPrint(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsPrint::CPrsPrint(const CPrsPrint& src) :CPrsNode(src.m_Symbol), m_string(src.m_string)
{
	m_expression = src.m_expression;
}
CPrsPrint::~CPrsPrint()
{
	Flush();
}

void CPrsPrint::Flush()
{
	m_expression = nullptr;
}
void CPrsPrint::Generate(stackGntInfo& stinfo)
{
	m_expression->Generate(stinfo);
	FVariant var = m_expression->getValue();
	var.toStr();
	
	// output
	std::wcout << (LPCWSTR)var.getSS() << L"\r\n";
}



void CPrsPrint::Parse()
{
	getNewSymbol();

	m_expression = std::make_shared<CPrsExpression>(m_Symbol);
	m_expression->Parse();

	auto tk = getSymbol();

	if ( tk.Token == rParen)
		tk = getNewSymbol();

	if (tk.Token != Semicol)
	{		
		THROW(L"';' expected");		
	}

	getNewSymbol();
}
