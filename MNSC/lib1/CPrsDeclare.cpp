#include "pch.h"
#include "CPrsNode.h"

///////////////////////////////////////////////////////////////////////////////////
CPrsDecaration::CPrsDecaration(CPrsSymbol& sym) :CPrsNode(sym)
{

}
void CPrsDecaration::Generate(stackGntInfo& stinfo)
{

}

void CPrsDecaration::Parse()
{
	SToken st = getSymbol();
	while(true)
	{
		switch (st.Token)
		{			
			case varSym:
			{
				getNewSymbol();
				CPrsDeclarVar		declar(m_Symbol);
				declar.Parse();
			}
			break;
			case funcSym:
			{
				getNewSymbol();
				CPrsDeclarFunction	declar(m_Symbol);
				declar.Parse();
			}
			break;			
			case noToken:
			case endSym:
			case beginSym:
				// Decarationは終了
				return;
			break;
			default:
			{				
				throw(st.Value);
			}
			break;
		}
		st = getSymbol();

	}

}
///////////////////////////////////////////////////////////////////////////////////
CPrsDeclarVar::CPrsDeclarVar(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsDeclarVar::CPrsDeclarVar(const CPrsDeclarVar& src) :CPrsNode(src), m_Ls(src.m_Ls)
{

}
void CPrsDeclarVar::Parse()
{
	SToken	st = getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();

	do
	{
		do
		{
			if (st.Token == Comma)
				st = getNewSymbol();

			if (st.Token == Ident)
			{
				SToken stIdent = st;

				symtbl.set(stIdent.Value, CSymbolTable::TYPE_LONG);
				FVariant var;
				var.setN(0);
				symtbl.setAt(stIdent.Value, var);

				m_Ls.push_back(stIdent.Value);

				st = getNewSymbol();

				if (st.Token == Becomes) // var a = 100;
				{
					st = getNewSymbol();
					if (st.Token == Number)
					{
						var.setN(_tstoi64(st.Value.c_str()));
						symtbl.setAt(stIdent.Value, var);
						st = getNewSymbol();
					}
					else if (st.Token == Quotation)
					{
						var.setS(st.Value);
						symtbl.setAt(stIdent.Value.c_str(), var);
						st = getNewSymbol();
					}
					else
					{
						THROW(L"'after equal expected");
					}
				}
			}
			else if (st.Token == Semicol)
				break;
			else if (st.Token == rParen)
				break;
			else
			{
				THROW(L"'Identifier expected");
			}
		} while (st.Token == Comma);

		if (st.Token == Semicol)
			st = getNewSymbol();
		else if (st.Token == rParen)
			break;
		else if (st.Token == Ident)
			THROW(L"',' expected");
		else
		{
			THROW(L"';' expected");
		}
	} while (st.Token == Ident);
}

///////////////////////////////////////////////////////////////////////////////////
CPrsDeclarFunction::CPrsDeclarFunction(CPrsSymbol& sym) :CPrsNode(sym)
{

}
void CPrsDeclarFunction::Parse()
{
	SToken	st = getSymbol();
	CSymbolTable& globalsymtbl = m_Symbol.getGlobalSymbolTable();
	wstring funcname;

	if (st.Token == Ident)
	{
		globalsymtbl.set(st.Value, CSymbolTable::TYPE_FUNC);
		funcname = st.Value;
	}
	else
		THROW(L"Identifier expected");

	st = getNewSymbol();

	if (st.Token == lParen)
	{
		// CPrsFunctionを作成してparseをかける。そして、SymbolTableへ登録する。
		// deleteはm_Symbolがうけもつ。

		std::shared_ptr<CPrsFunction> func = m_Symbol.RegistMasterFunc(funcname);
		if (func) func->Parse();

	}
	else
		THROW(L"'(' expected");
}
