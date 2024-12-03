#include "pch.h"
#include "CPrsNode.h"
#include "FVariantArray.h"


class SMethod
{
	public:
		SMethod():Token(0){}

		FVariant value;
		int		Token;
};

////////////////////////////Expression////////////////////////////////////////////////
class CSubTerm
{
	public:
		CSubTerm(std::shared_ptr<CPrsTerm> p, int Token = noToken) { term = p; token = Token; }
		CSubTerm(const CSubTerm& src);
		~CSubTerm() { term = nullptr; }
	public:
		std::shared_ptr<CPrsTerm>	term;
		int			token;
};

CSubTerm::CSubTerm(const CSubTerm& src)
{
	token = src.token;	
	term = src.term;
}

// ////////////////////////////////////////////////////////////////////////////////////////
CPrsExpression::CPrsExpression(CPrsSymbol& sym) :CPrsNode(sym)
{
	m_bMinus = FALSE;
}
CPrsExpression::CPrsExpression(const CPrsExpression& src) :CPrsNode(src.m_Symbol)
{	
	m_Ls = src.m_Ls;
	m_Value = src.m_Value;
	m_bMinus = src.m_bMinus;
}
CPrsExpression::~CPrsExpression()
{
	Flush();
}
void CPrsExpression::Flush()
{	
	m_Ls.clear();
}
void CPrsExpression::Generate(stackGntInfo& stinfo)
{
	SMethod		sm;
	
	for(auto& it : m_Ls) 
	{
		FVariant value;
		CSubTerm* subterm = it.get();

		if (subterm->term)
		{
			subterm->term->Generate(stinfo);
			value = subterm->term->getValue();
		}

		switch (subterm->token)
		{
			case Plus:
			case Minus:
			
				sm.Token = subterm->token;
			break;
			case noToken:
			{
				switch (sm.Token)
				{
				case Plus:
					sm.value = sm.value + value;
					break;
				case Minus:
					sm.value = sm.value - value;
					break;
				
				case noToken:
					sm.value = value;
					break;
				}
			}
			break;
		}
	}

	m_Value = sm.value;

	if ( m_bMinus )
	{
		auto vt = m_Value.vt;
		if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || vt == VT_INT)
			m_Value.setN(-m_Value.getN());
		else if (vt == VT_R8 || vt == VT_R4)
			m_Value.setD(-m_Value.getD());
	}


	if ( next_ )
	{
		auto func = dynamic_cast<CPrsFactor*>(next_.get());

		if ( func )
		{
			func->SetToken(IdentVarFunc, m_Value);
			func->Generate(stinfo);
			m_Value = func->getValue(); // function pointer
		}
		else
		{
			auto func2 = dynamic_cast<CPrsVarFunction*>(next_.get());

			if ( func2 )
			{
				func2->setgetValue(m_Value);
				func2->Generate(stinfo);
				m_Value = func2->Return();
			}
		}


	}
}
void CPrsExpression::Parse()
{
	DEBUG_PARSE(CPrsExpression)
	if (getSymbol().Token == Minus)
	{
		m_bMinus = true;
		getNewSymbol();
	}

	// term
	auto term = std::make_shared<CPrsTerm>(m_Symbol);
	term->Parse();

	m_Ls.push_back(std::make_shared<CSubTerm>(term));

	auto st = getSymbol();

	while (st.Token == Plus || st.Token == Minus ) //|| st.Token == OROR)
	{
		m_Ls.push_back(std::make_shared<CSubTerm>(nullptr, st.Token));

		// term()
		getNewSymbol();
		term = std::make_shared<CPrsTerm>(m_Symbol);
		term->Parse();

		m_Ls.push_back(std::make_shared<CSubTerm>(term));
		st = getSymbol();
	}

	st = getSymbol();


}
/////////////////////////////////Trem///////////////////////////////////////////////////////
class CSubFactor
{
public:
	CSubFactor(std::shared_ptr<CPrsFactor> p, int Token = noToken) { factor = p; token = Token; }
	CSubFactor(const CSubFactor& src)
	{
		token = src.token;		
		factor = src.factor;
	}
	~CSubFactor()
	{
		factor = nullptr;
	}
public:
	std::shared_ptr<CPrsFactor> factor;
	int			token;
};

/////////////////////////////////////////////////////////////////////////////////////

CPrsTerm::CPrsTerm(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsTerm::CPrsTerm(const CPrsTerm& src) :CPrsNode(src.m_Symbol)
{
	m_Value = src.m_Value;
	m_Ls = src.m_Ls;
}
CPrsTerm::~CPrsTerm()
{
	Flush();
}

void CPrsTerm::Flush()
{
	m_Ls.clear();
}
void CPrsTerm::Generate(stackGntInfo& stinfo)
{
	SMethod		sm;

	for(auto& it : m_Ls)
	{
		FVariant value;
		CSubFactor* subfactor = it.get();

		if (subfactor->factor)
		{
			subfactor->factor->Generate(stinfo);
			value = subfactor->factor->getValue();
		}


		switch (subfactor->token)
		{
			case Times:
			case Slash:
			case Percent:
			//case Equal:
			
				sm.Token = subfactor->token;
				break;
			case noToken:
			{
				switch (sm.Token)
				{
				case Times:
					sm.value = sm.value * value;
					break;
				case Slash:
					sm.value = sm.value / value;
					break;
				case Percent:
					sm.value = sm.value % value;
					break;
				/*case Equal:
					sm.value.setBL(sm.value == value ? VARIANT_TRUE : VARIANT_FALSE);
					break;
				case notEqual:
					sm.value.setBL(sm.value == value ? VARIANT_FALSE : VARIANT_TRUE);
					break;*/
				
				case noToken:
					sm.value = value;
					break;
				}
			}
			break;
		}
	}

	m_Value = sm.value;
}
void CPrsTerm::Parse()
{
	DEBUG_PARSE(CPrsTerm)

	// factor
	auto factor = std::make_shared<CPrsFactor>(m_Symbol);
	factor->Parse();

	m_Ls.push_back(std::make_shared<CSubFactor>(factor));

	SToken	st = getSymbol();

	while (st.Token == Times || st.Token == Slash || st.Token == Percent ) //|| st.Token == Equal || st.Token == notEqual)
	{
		int TokenMethod = st.Token;
		
		m_Ls.push_back(std::make_shared<CSubFactor>(nullptr, TokenMethod));

		// factor
		st = getNewSymbol();
		factor = std::make_shared<CPrsFactor>(m_Symbol);
		factor->Parse();

		m_Ls.push_back(std::make_shared<CSubFactor>(factor));

		st = getSymbol();
	}
}

//////////////////////Factor////////////////////////////////////////////////////////////

CPrsFactor::CPrsFactor(CPrsSymbol& sym) :CPrsNode(sym)
{
	m_Token = 0;
}
CPrsFactor::CPrsFactor(const CPrsFactor& src) :CPrsNode(src.m_Symbol)
{	
	m_Value = src.m_Value;
	m_IdentName = src.m_IdentName;
	m_Token = src.m_Token;
	m_dot_next = src.m_dot_next;

	m_node = factory(src.m_node.get(), m_Token);
}
CPrsFactor::~CPrsFactor()
{
	Flush();
}

void CPrsFactor::Flush()
{
	m_node = nullptr;
	m_dot_next = nullptr;
}

void CPrsFactor::Parse()
{
	DEBUG_PARSE(CPrsFactor)

	SToken	st = getSymbol();

	m_Token = st.Token;

	auto& localtbl = m_Symbol.getSymbolTable();
	bool bfunction_pointer = false;


	switch (m_Token)
	{
		case Ident:
		{
			auto nst = m_Symbol.getNewPeekSymbol();
			bool IsGlobal = !localtbl.IsContain(st.Value);
			bfunction_pointer = ((nst.Token == Semicol || nst.Token == rSquare || nst.Token == rParen || nst.Token == Comma) && IsGlobal);
			if (bfunction_pointer)
			{
				ParseFunctionPointer(st);
			}
			else if ( nst.Token == lParen )
			{
				ParseIdent(st);
			}
			
			m_IdentName = st.Value;
		}
		break;
		case Number:
		{
			__int64	value;
			ParseNumber(st, value);
			m_Value.setN(value);
		}
		break;
		case Float:
			m_Value.setD(_tstof(st.Value.c_str()));			
		break;
		
		case lParen:
		{
			ParselParen(st);

			auto st2 = getSymbol();
			if (st2.Token == Dot)
			{				
				getNewSymbol();
				m_dot_next = std::make_shared<CPrsVarFunction>(m_Symbol);
				m_dot_next->Parse();
				return;
			}
		}
		break;

		case Dot:
		{
			auto pre = this->m_Symbol.getPreSymbol();
			m_IdentName = pre.Value;

			getNewSymbol();
			m_node = std::make_shared<CPrsVarFunction>(m_Symbol);
			m_node->Parse();

			
		}
		break;
		case Quotation:
			m_Value.setS(st.Value);
		break;
		case Plusplus:
		{
			//ex. a++ 
			auto pre = this->m_Symbol.getPreSymbol();
			m_IdentName = pre.Value;
		}
		break;
		case lSquare:			
			ParselSquare(st);		// making FVariantArray
		break;	
		case lBracket:			
			ParselBracket(st);		// making FVariantMap
		break;
		case trueSym:
			m_Value.setBL(true);
		break;
		case falseSym:
			m_Value.setBL(false);
		break;

		default:
		{			
			std::wstringstream sm;
			sm << L"CPrsFactor::Parse() err token:" << m_Token << L" " << st.Value;
			THROW(sm.str());
		}
		break;
	}

	st = getSymbol();

	if ( st.Token == Equal || st.Token == notEqual)
		return;

	if ( st.Token != Semicol )
		st = getNewSymbol();

	// ここでstはexpressionの次の段階

	int flg = 0;
	if (m_Token == Ident )
	{
		if (st.Token == Dot)
		{
			getNewSymbol();
			m_Token = IdentVarFunc;
			ParseVarFunction(st);
		}
		else if (st.Token == lParen)
		{			
			m_Token = IdentVarFunc;
			ParseVarFunction(st);
		}
		else if (st.Token == lSquare)
		{
			m_Token = IdentArray;
			ParseIdentSquare(st);
			flg = 1;
		}
		else if (st.Token == lBracket)
		{
			m_Token = IdentVarDic;
			ParseVarFunction(st);
		}

		else if (bfunction_pointer)
		{
			//getNewSymbol();
			m_Token = funcPointerSym;
					
			m_node = factory(nullptr, funcPointerSym);
			m_node->Parse();
		}
	}

	auto st2 = getSymbol();

	if (st2.Token == rParen)
	{
		st2 = m_Symbol.getNewPeekSymbol();		
	}
	else if (st2.Token == Dot)
	{
		int i = 0;
		CPrsVarFunction* nnext = nullptr;
		while(st2.Token == Dot)
		{
			getNewSymbol();
			auto next = std::make_shared<CPrsVarFunction>(m_Symbol);
			next->Parse();

			if (i == 0)
				m_dot_next = next;
			else if (i > 0)
				nnext->next_ = next;
			
			st2 = getSymbol();

			nnext = static_cast<CPrsVarFunction*>(next.get());
			i++;
		}		
	}
	else if (st2.Token == lSquare && flg == 1)
	{		
		auto& target = this->m_dot_next;
		int i = 0;
		CPrsIdentArray* nnext = nullptr;
		while(st2.Token == lSquare)
		{
			auto next = factory(nullptr, m_Token);
			next->Parse();

			if ( i == 0 )
				m_dot_next = next;
			else if ( i > 0)
				nnext->next_ = next;

			st2 = getSymbol();

			nnext = static_cast<CPrsIdentArray*>(next.get());
			i++;			
		}
	}
}
void CPrsFactor::ParseVarPoint(SToken& st)
{
	ATLASSERT(m_Token == IdentVarPoint);

	
}
void CPrsFactor::ParseVarFunction(SToken& st)
{
	m_node = factory(nullptr, m_Token);
	m_node->Parse();
}
void CPrsFactor::ParseIdentSquare(SToken& st)
{
	m_node = factory(nullptr, m_Token);
	m_node->Parse();
}
void CPrsFactor::ParselSquare(const SToken& st)
{
	SToken st2;
	
	st2 = getNewSymbol();
	if (st2.Token != rSquare)
	{
		auto nd = factory(nullptr, m_Token);
		nd->Parse();

		m_node = nd;
		auto pre = nd; 

		st2 = getSymbol();
		while (st2.Token == Comma)
		{
			getNewSymbol();

			nd = factory(nullptr, m_Token);				
			nd->Parse();

			CPrsExpression* pre2 = dynamic_cast<CPrsExpression*>(pre.get());
			_ASSERT(pre2);
			pre2->next_ = nd;

			pre = nd;
			st2 = getSymbol();
		}
	}
	
	st2 = getSymbol();
	if (st2.Token != rSquare)
		THROW(L", ERROR");
}
void CPrsFactor::ParselBracket(const SToken& st)
{
	// { "key": 1, "key2" : 2 ... };

	SToken st2;

	st2 = getNewSymbol();
	if (st2.Token != rBracket)
	{
		wstring key = st2.Value;

		st2 = getNewSymbol();
		if (st2.Token != Colon )
			THROW(L"map key : error");

		getNewSymbol();
		auto nd = factory(nullptr, m_Token);
		nd->Parse();

		CPrsExpression* nd2 = dynamic_cast<CPrsExpression*>(nd.get());
		nd2->m_Value.setS(key);

		m_node = nd;
		auto pre = nd2;

		st2 = getSymbol();
		while (st2.Token == Comma)
		{
			st2 = getNewSymbol();
			key = st2.Value;
			st2 = getNewSymbol();
			if (st2.Token != Colon)
				THROW(L"map key : error");

			getNewSymbol();
			nd = factory(nullptr, m_Token);
			nd->Parse();

			CPrsExpression* nd21 = dynamic_cast<CPrsExpression*>(nd.get());
			_ASSERT(nd21);
			nd21->m_Value.setS(key);
			pre->next_ = nd;

			pre = nd21;
			st2 = getSymbol();
		}
	}

	st2 = getSymbol();
	if (st2.Token != rBracket)
		THROW(L", ERROR");
}
void CPrsFactor::ParselParen(SToken& st)
{
	getNewSymbol();
	m_node = factory(nullptr, m_Token);//new CPrsExpression( symbol );
	m_node->Parse();
	auto st2 = getSymbol();
}
void CPrsFactor::TestParseFunctionPointer()
{
	//getNewSymbol();
	m_node = factory(nullptr, ParameterSym);
	m_node->Parse();
	auto st2 = getSymbol();


}
void CPrsFactor::ParseFunctionPointer(SToken& st)
{
	int type = 0;

	// グローバル変数
	CSymbolTable& globalsymtbl = m_Symbol.getGlobalSymbolTable();
	if (globalsymtbl.findTable(st.Value, type))
	{
		if (type == CSymbolTable::TYPE_FUNC)
		{
			st.Token = funcPointerSym;
		}
	}
	else
		THROW(L"function pointer error");

}
int CPrsFactor::ParseIdent(SToken& st)
{
	int type = 0;

	// Global value
	CSymbolTable& globalsymtbl = m_Symbol.getGlobalSymbolTable();
	BOOL found_global = globalsymtbl.findTable(st.Value, type);
	if (found_global)
	{
		if (type == CSymbolTable::TYPE_FUNC)
		{
			m_Token = IdentFunc;
			int typ = ParseIdentFunc(st);	
			return typ;
		}
		else if (type == CSymbolTable::TYPE_BUILTIN_FUNC)
		{
			m_Token = BuiltinFunc;
			ParseBuiltinFunc(st);

		}
	}	


	// Loacl value
	CSymbolTable& localsymtbl = m_Symbol.getSymbolTable();
	BOOL found_local = localsymtbl.findTable(st.Value, type);
	if (found_local)
	{
		if (type == CSymbolTable::TYPE_LONG)
		{
			// do nothing
		}
		else if (type == CSymbolTable::TYPE_FUNC)
		{
			m_Token = IdentFunc;
			int typ = ParseIdentFunc(st);
			//if (typ == 2)
			//{
			//	// function pointer
			//	m_Token = IdentFunc;
			//	return 2;
			//}

		}
	}	


	if (!found_global && !found_local)
	{
		wstring err = L"not def:";
		err += st.Value;		
		throw(err);
	}
	return 0;
}
void CPrsFactor::ParseNumber(SToken& st, __int64& outValue)
{
	outValue = _tstoi64(st.Value.c_str());
}
int CPrsFactor::ParseIdentFunc(SToken& st)
{
	auto funcName = st.Value;

	auto nst = m_Symbol.getNewPeekSymbol(); //getNewSymbol();

	if ( nst.Token == lParen)
	{
		getNewSymbol();
		m_node = factory(nullptr, m_Token); //new CPrsParameters( symbol );
		m_node->Parse();
	}
	return 0;
}
int CPrsFactor::ParseBuiltinFunc(SToken& st)
{
	auto funcName = st.Value;

	auto nst = m_Symbol.getNewPeekSymbol(); //getNewSymbol();

	if (nst.Token == lParen)
	{
		//getNewSymbol();
		m_node = factory(nullptr, m_Token);
		m_node->Parse();
	}
	return 0;
}

void CPrsFactor::Generate(stackGntInfo& stinfo)
{
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();
	CSymbolTable& symtblG = m_Symbol.getGlobalSymbolTable();
	switch (m_Token)
	{
		case Ident:
		case IdentVarFunc:
		{
			FVariant vSelf;
			

			if (!m_IdentName.empty())
			{
				if (0 != symtbl.getAt(m_IdentName, vSelf))
				{
					if (0 != symtblG.getAt(m_IdentName, vSelf))
					{
						wstring err = L"not def Idnet in Generate: ";
						err += m_IdentName;
						throw(err);
					}
				}
			}
			else
				vSelf = m_Value;

			if (m_node)
			{
				CPrsVarFunction* varfunc =  dynamic_cast<CPrsVarFunction*>(m_node.get());
			
				if ( varfunc )
				{
					FVariant temp(vSelf);
					varfunc->setgetValue(temp);
					varfunc->Generate(stinfo); // varの関数
					m_Value = varfunc->Return();
				}
				else if (dynamic_cast<CPrsParameters*>(m_node.get()) && vSelf.vt == VT_UNKNOWN)
				{
					// a[0](); の()部分の実行
					auto prmfunc = dynamic_cast<CPrsParameters*>(m_node.get());
					prmfunc->Generate(stinfo);
				
					auto vfunc = dynamic_cast<IVARIANTAbstract*>(vSelf.punkVal);
					if (FVariantType::FUNCTIONPOINTER == vfunc->TypeId() )
					{
						auto& ar = prmfunc->getParam();

						auto v = std::make_shared<FVariant[]>(ar.size());
						int i = 0;
						for(auto& it : ar )
						{
							v[i++] = it;
						}

						m_Value = vfunc->Invoke(L"NONAME", (VARIANT*)&v[0], (int)ar.size());
					}
				}
				else
					THROW(L"CPrsFactor::Generate error");

			}
			else
			{
				if ( vSelf.vt == VTF_FUNC)
				{	
					auto funcnm = m_IdentName;
					auto ifunc = new IVARIANTFunctionImp();					
					auto func = m_Symbol.CreateFunc(funcnm);
					func->Generate(stinfo);
					ifunc->SetItem(func);

					//::VariantClear(&m_Value);
					m_Value.clear_init();

					m_Value.vt = VT_UNKNOWN;
					m_Value.punkVal = ifunc;
				}
				else
					m_Value = vSelf;
			}


			//::VariantClear(&vSelf);
		}
		break;

		case Number:
		{
			//int idx = m_Value.getN();

			
		}
		break;
	
		case Float:
		case Quotation:
			//TRACE( "%s\n", (LPCTSTR)m_IdentName );
			
		break;
		case lParen:
			if (m_node)
			{
 				m_node->Generate(stinfo);
				_ASSERT(dynamic_cast<CPrsExpression*>(m_node.get()));

				m_Value = dynamic_cast<CPrsExpression*>(m_node.get())->getValue();
			}
		break;
		case Dot:
		{
			if (m_node)
			{
				FVariant var;
				if (0 != symtbl.getAt(m_IdentName, var))
				{
					if (0 != symtblG.getAt(m_IdentName, var))
					{
						wstring err = L"not def Idnet in Generate: ";
						err += m_IdentName;
						throw(err);
					}
				}





				auto func = static_cast<CPrsVarFunction*>(m_node.get());

				func->setgetValue(var);

				func->Generate(stinfo);

			}
		}
		break;
		case IdentFunc:
		{
			if (m_node)
			{
				auto param = std::make_unique<CPrsParameters>(*((CPrsParameters*)m_node.get()));
				auto func = m_Symbol.CreateFunc(m_IdentName);

				param->Generate(stinfo);
				func->SetParameters(*param);
				func->Generate(stinfo);
				m_Value = func->getValue();

			
				func = nullptr;
			}
		}
		break;
		case BuiltinFunc:
		{
			m_node->Generate(stinfo);

			m_Value = ((CPrsBuiltinFunction*)m_node.get())->Return();
		}
		break;


		case Plusplus:
		{
			FVariant var;
			if (0 != symtbl.getAt(m_IdentName, var))
			{
				if (0 != symtblG.getAt(m_IdentName, var))
				{
					wstring err = L"not def Idnet in Generate: ";
					err += m_IdentName;
					throw(err);
				}
			}

			var.llVal += 1;
			m_Value = var;
		}
		break;
		case lSquare:
		{
			// generate FVariantArray, 
			std::vector<FVariant> data;

			auto it = m_node;
			while(it) 
			{
				it->Generate(stinfo);
				CPrsExpression* exp = (CPrsExpression*)it.get();
				data.push_back(exp->getValue());

				if (exp->next_)
					it = exp->next_;
				else
					break;
			}
			m_Value.setAr(data);
		}
		break;
		case funcPointerSym:
		{
			m_node->Generate(stinfo);
		
			CPrsIdentFunctionPointer* x = (CPrsIdentFunctionPointer*)m_node.get();

			m_Value = x->Value();
		}
		break;
		case lBracket:
		{
			// generate FVariantMap
			std::map<wstring,FVariant> map;

			auto it = m_node;
			while (true)
			{
				CPrsExpression* exp = (CPrsExpression*)it.get(); 

				if ( exp )
				{
					FVariant key;
					key = exp->getValue(); //>m_Value;

					if ( key.vt = VT_BSTR && key.bstrVal )
					{
						exp->Generate(stinfo);			
						map[key.getSS()] = exp->getValue();
					}	

					if (exp->next_)
						it = exp->next_;
					else
						break;
				}
				else
					break;
			}
			m_Value.setMap(map);
		}
		break;
		case IdentArray:
		{
			m_node->Generate(stinfo);
			FVariant idx = ((CPrsIdentArray*)m_node.get())->Value();

			FVariant var;
			if (0 != symtbl.getAt(m_IdentName, var))
			{
				if (0 != symtblG.getAt(m_IdentName, var))
				{
					wstring err = L"not def Idnet in Generate: " + m_IdentName;				
					throw(err);
				}
			}

			if ( var.vt == VT_EMPTY )
			{
				std::wstring s = L"IdentArray err: " + m_IdentName;
				throw(s);
			}

			if (var.vt == VT_UNKNOWN )
			{
				IVARIANTArray* x = (IVARIANTArray*)var.punkVal;

				if ( x->TypeId() == FVariantType::ARRAY)
				{
					VARIANT v;
					::VariantInit(&v);

					if (idx.vt != VT_I8 && idx.vt != VT_I4 && idx.vt != VT_I2)
						THROW(L"array idx err");
					
					if (x->Get((int)idx.getN(), &v))
						m_Value = v;
					else
					{
						std::wstringstream err;
						err << L"not found index: " << idx.llVal;
						THROW(err.str());
					}

				}
				else if (x->TypeId() == FVariantType::MAP)
				{
					IVARIANTMap* m = (IVARIANTMap*)var.punkVal;

					if (idx.vt != VT_BSTR)
						THROW(L"map key err");

					wstring key = idx.bstrVal;

					VARIANT v;
					::VariantInit(&v);
					if ( m->GetItem(key, &v))
						m_Value = v;
					else
					{
						wstring err = L"not found key: " + key;
						THROW(err);
					}
				}
				else
					THROW(L"Generate Square err");
			}
			else if ( var.vt == VT_BSTR)
			{
				BSTR s = var.bstrVal;
				ULONG len = ::SysStringLen(s);

				if ( idx.llVal < len )
				{
					WCHAR cb[2]={};
					cb[0] = s[idx.llVal];
					m_Value.setS(cb);
				}
				else
					THROW(L"string pos is over");
			}
			else
				THROW(L"VT is invalid");
		}
		break;
		

	}



	
	if ( m_dot_next )
	{
		CPrsVarFunction* pf = dynamic_cast<CPrsVarFunction*>(m_dot_next.get());
		if ( pf )
		{
			pf->setgetValue(m_Value);
			pf->Generate(stinfo);
			m_Value = pf->Return();
		}
		else
		{
			CPrsIdentArray* pf2 = dynamic_cast<CPrsIdentArray*>(m_dot_next.get());
			pf2->setgetValue(m_Value);
			pf2->Generate(stinfo);
			m_Value = pf2->Value();
		}

	}
}

std::shared_ptr<CPrsNode> CPrsFactor::factory(CPrsNode* src, int type, LPCTSTR funcName)
{
	std::shared_ptr<CPrsNode> node;
	switch (type)
	{
		case IdentFunc:
		case IdentExport:
			if (src)
				node = std::make_shared<CPrsParameters>(*(CPrsParameters*)src);
			else
				node = std::make_shared<CPrsParameters>(m_Symbol);
		break;
		case lParen:	// (
		case lBracket:	// {
		case lSquare:	// [
			if (src)
				node = std::make_shared<CPrsExpression>(*(CPrsExpression*)src);
			else
				node = std::make_shared<CPrsExpression>(m_Symbol);
		break;
		case IdentVarFunc:
			if (src)
				node = std::make_shared <CPrsVarFunction>(*(CPrsVarFunction*)src);
			else
				node = std::make_shared <CPrsVarFunction>(m_Symbol);
		break;
		case IdentArray:		
			if (src)
				node = std::make_shared <CPrsIdentArray>(*(CPrsIdentArray*)src);
			else
				node = std::make_shared <CPrsIdentArray>(m_Symbol);
		break;
		case funcPointerSym:
			if (src)
				node = std::make_shared <CPrsIdentFunctionPointer>(*(CPrsIdentFunctionPointer*)src);
			else
				node = std::make_shared <CPrsIdentFunctionPointer>(m_Symbol);
		break;
		case BuiltinFunc:
			if (src)
				node = std::make_shared <CPrsBuiltinFunction>(*(CPrsBuiltinFunction*)src);
			else
				node = std::make_shared <CPrsBuiltinFunction>(m_Symbol);

		break; 
		case ParameterSym:
			if (src)
				node = std::make_shared <CPrsParameters>(*(CPrsParameters*)src);
			else
				node = std::make_shared <CPrsParameters>(m_Symbol);
		break;
	}
	return node;
}
