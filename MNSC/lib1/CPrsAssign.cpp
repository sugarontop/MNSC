#include "pch.h"
#include "CPrsNode.h"
// CPrsAssign

//see CPrsStatment::factoryParse

CPrsAssign::CPrsAssign(CPrsSymbol& sym) :CPrsNode(sym), m_const(false)
{
	
}
CPrsAssign::CPrsAssign(const CPrsAssign& src) :CPrsNode(src.m_Symbol)
{
	m_expression = src.m_expression;
	m_AssignName = src.m_AssignName;
	m_const = src.m_const;
}
CPrsAssign::~CPrsAssign()
{
	Flush();
}
void CPrsAssign::Flush()
{
	m_expression = nullptr;
}

void CPrsAssign::HParse()
{
	DEBUG_PARSE(CPrsAssign)
	SToken st = m_Symbol.getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();
	CSymbolTable& symtblG = m_Symbol.getGlobalSymbolTable();


	int type;
	bool bPlusplus = false;
	if (symtbl.findTable(st.Value, type))
	{
		m_AssignName = st.Value;

//gAssign = m_AssignName;

		if (type == CSymbolTable::TYPE_OBJECT)
		{
			m_expression = std::make_shared<CPrsExpression>(m_Symbol); // TYPE_OBJECT‚Ìê‡‚Í''‚ð‚Æ‚Î‚·‚±‚Æ‚ª‚Å‚«‚é
			m_expression->Parse();
		}
		else
		{
			st = getNewSymbol();
			if (st.Token == Becomes)
			{				
				getNewSymbol();
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
			}
			else if ( st.Token == Plusplus )
			{
				//st = getNewSymbol();
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
				bPlusplus = true;
			}
			else if (st.Token == Dot)
			{
				//st = getNewSymbol();
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
			}
			
		}


		if (getNewPeekSymbol().Token == Semicol)
			getNewSymbol();

		st = getSymbol();
		if ( !bPlusplus )
		{
			if (st.Token != Semicol)
			{
				auto x = getSymbol().Token;

				if (x != Semicol)
					THROW(L"';' expected or unknown token");
			}
			getNewSymbol();
		}
		else
		{
			
			if (st.Token == Semicol)
				getNewSymbol();
		}

	}
	else if (symtblG.findTable(st.Value, type))
	{
		m_AssignName = st.Value;

		if (type == CSymbolTable::TYPE_OBJECT)
		{
			m_expression = std::make_shared<CPrsExpression>(m_Symbol);
			m_expression->Parse();
		}
		else
		{
			auto st2 = getNewPeekSymbol();
			if (st2.Token == Becomes)
			{
				getNewSymbol();
				getNewSymbol();
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
			}
			else
			{
				// Becomes less formula
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
			}
		}

		if (getNewPeekSymbol().Token == Semicol)
			getNewSymbol();

		if (getSymbol().Token != Semicol)
		{
			THROW(L"';' expected or unknown token");
		}
		getNewSymbol();
	}
	else
	{
		wstring ss;		
		ss = st.Value;
		ss += L" :not def"; 
		throw(ss);
	}	
}
void CPrsAssign::HGenerate(stackGntInfo& stinfo)
{
	if (m_expression)
	{
		m_expression->Generate(stinfo);

		FVariant var = m_expression->getValue();

		if (m_const)
			var.lock();


		if ( var.vt != VT_EMPTY)
		{
			CSymbolTable& symtbl = m_Symbol.getSymbolTable();
			if (0 != symtbl.updateAt(m_AssignName, var))
			{
				CSymbolTable& symtblG = m_Symbol.getGlobalSymbolTable();

				if (0 != symtblG.updateAt(m_AssignName, var))
				{
					wstring er;
					er = L"not find symtable " + m_AssignName;
					throw(er);
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// CPrsAssignSquare

CPrsAssignSquare::CPrsAssignSquare(CPrsSymbol& sym) :CPrsAssign(sym)
{
	m_expressionInBracket = nullptr;
}
CPrsAssignSquare::CPrsAssignSquare(const CPrsAssignSquare& src) :CPrsAssign(src)
{
	m_expressionInBracket = src.m_expressionInBracket;
}
CPrsAssignSquare::~CPrsAssignSquare()
{
	Flush();
}
void CPrsAssignSquare::Flush()
{
	CPrsAssign::Flush();
	
	m_expressionInBracket = nullptr;

	m_ar.clear();

}
static wstring Format( LPCTSTR format, const wstring&s1, const wstring& s2)
{
	WCHAR cb[256];
	StringCbPrintf( cb,_countof(cb), format, s1.c_str(), s2.c_str() );

	return cb;
}
static wstring Format2(LPCTSTR format, const wstring& s1, __int64 s2)
{
	WCHAR cb[256];
	StringCbPrintf(cb, _countof(cb), format, s1.c_str(), s2);

	return cb;
}
void CPrsAssignSquare::Generate(stackGntInfo& stinfo)
{
	if (m_expression && m_expressionInBracket)
	{
		CSymbolTable& symtbl = m_Symbol.getSymbolTable();

		m_expressionInBracket->Generate(stinfo);
		m_expression->Generate(stinfo);

		auto varSb = m_expression->getValue();
		auto vkey = m_expressionInBracket->getValue();

		FVariant var;
		symtbl.getAt(m_AssignName, var);

		if (var.VT() == VT_UNKNOWN)
		{
			var.setAr(vkey, varSb);
		}
		else
		{
			wstring sp = var.getSS();
			std::wstring varS = varSb.bstrVal;
			int idx = (int)vkey.llVal;
			if (idx < sp.length() && varS.length() > 0)
			{
				sp[idx] = varS[0];
				var.setS(sp);
			}
			symtbl.setAt(m_AssignName, var);
		}
	}
	else if (!m_ar.empty())
	{
		CSymbolTable& symtbl = m_Symbol.getSymbolTable();

		FVariant var;
		var.setAr(m_ar);
		symtbl.setAt(m_AssignName, var);
	}

}
void CPrsAssignSquare::Parse()
{
	const SToken& st = m_Symbol.getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();


	int type;
	if (symtbl.findTable(st.Value, type))
	{
		m_AssignName = st.Value;

		if (type == CSymbolTable::TYPE_OBJECT)
		{
			wstring ss = L"type is Ident: ";
			ss += m_AssignName;
			throw(ss);
		}
		else
		{
			if (getNewSymbol().Token == lSquare)
			{
				if (getNewSymbol().Token == rSquare)
				{
					// var a;
					// a[] = ["furuya", "xxxxxx", "yokohama"];

					_ASSERT(m_expressionInBracket == NULL);
					if (getNewSymbol().Token == Becomes)
					{
						getNewSymbol();
						ParseAutoAr();
					}
					else
						THROW(L"'=' expected");

				}
				else
				{
					m_expressionInBracket = std::make_shared<CPrsExpression>(m_Symbol);
					m_expressionInBracket->Parse();

					if (getNewSymbol().Token == Becomes)
					{
						getNewSymbol();
						m_expression = std::make_shared<CPrsExpression>(m_Symbol);
						m_expression->Parse();
					}
					else
						THROW(L"'=' expected");

				}
			}
			else
				THROW(L"'[' expected");


		}


		if (getSymbol().Token != Semicol)
		{
			THROW(L"';' expected or unknown token");
			
		}
		getNewSymbol();
	}
	else
	{
		wstring s1 = st.Value;
		s1 += L" :NotDef";		
		throw(s1);
	}
}

void CPrsAssignSquare::ParseAutoAr()
{
	SToken	st = getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();

	if (st.Token != lSquare)
		THROW(L"'[' expected");

	st = getNewSymbol();

	do
	{

		if (st.Token == Comma)
			st = getNewSymbol();

		if (st.Token == Quotation || st.Token == Number || st.Token == Float)
		{

			FVariant var;

			if (st.Token == Quotation)
				var.setS(st.Value);
			else if (st.Token == Number)
				var.setN(_tstoi64(st.Value.c_str()));
			else if (st.Token == Float)
				var.setD(_tstof(st.Value.c_str()));

			m_ar.push_back(var);

			st = getNewSymbol();

			if (st.Token == rSquare) // ']'
			{
				st = getNewSymbol();
				break;
			}
		}
		else
		{
			THROW(L" '...' or num or float expected");
			break;
		}
	} 
	while (st.Token == Comma);
}




//////////////////////////////////////////////////////////////////////////////////
// CPrsAssignPlusEqual 

CPrsAssignPlusEqual::CPrsAssignPlusEqual(CPrsSymbol& sym) :CPrsAssign(sym)
{
	m_Token = 0;
}
CPrsAssignPlusEqual::CPrsAssignPlusEqual(const CPrsAssignPlusEqual& src) :CPrsAssign(src), m_Token(src.m_Token)
{

}
CPrsAssignPlusEqual::~CPrsAssignPlusEqual()
{
	Flush();
}
void CPrsAssignPlusEqual::Parse()
{
	const SToken& st = m_Symbol.getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();


	int type;
	if (symtbl.findTable(st.Value, type))
	{
		m_AssignName = st.Value;

		if (type == CSymbolTable::TYPE_OBJECT)
		{
			m_expression = std::make_shared<CPrsExpression>(m_Symbol); // TYPE_OBJECT‚Ìê‡‚Í''‚ð‚Æ‚Î‚·‚±‚Æ‚ª‚Å‚«‚é
			m_expression->Parse();
		}
		else
		{
			int token = getNewSymbol().Token;
			if (token == PlusEqual || token == MinusEqual)
			{
				m_Token = token;
				getNewSymbol();
				m_expression = std::make_shared<CPrsExpression>(m_Symbol);
				m_expression->Parse();
			}
			else if (token == Plusplus || token == Minusminus)
			{
				m_Token = token;
				getNewSymbol();
			}
		}

		auto st2 = getSymbol();

		if ( st2.Token == rParen )
		{

		}
		else if (st2.Token == Semicol)
		{
			getNewSymbol(); 			
		}
		else
			THROW(L"';' expected or unknown token");
		
	}
	else
	{
		wstring ss = st.Value;
		ss += L" :NotDef";
		
		throw(ss);
	}
}

void CPrsAssignPlusEqual::Generate(stackGntInfo& stinfo)
{
	FVariant var(1);
	FVariant varx(-1);

	if (m_expression)
	{
		m_expression->Generate(stinfo);

		var = m_expression->getValue();
	}

	FVariant varsource;
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();
	symtbl.getAt(m_AssignName, varsource);


	switch (m_Token)
	{
		case PlusEqual:
			varsource += var;
			break;
		case MinusEqual:
			varsource += (var*varx);
		break;
		case Plusplus:
			varsource += var;
			break;
		case Minusminus:
			varsource += varx;
			break;
	}

	symtbl.setAt(m_AssignName, varsource);
}


std::string w2a(std::wstring ws)
{
	int nSize = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
	if (nSize > 0)
	{
		std::string str(nSize, '\0');
		WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, &str[0], nSize, NULL, NULL);
		return str;
	}
	return std::string();

}

