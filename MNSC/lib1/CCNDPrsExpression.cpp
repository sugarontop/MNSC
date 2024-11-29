#include "pch.h"
#include "CPrsNode.h"
#include "FVariantArray.h"

class SCNDMethod
{
public:
	SCNDMethod() :Token(0) {}

	FVariant value;
	int		Token;
};

////////////////////////////Expression////////////////////////////////////////////////
class CCNDSubTerm
{
public:
	CCNDSubTerm(std::shared_ptr<CCNDPrsTerm> p, int Token = noToken) { term = p; token = Token; }
	CCNDSubTerm(const CCNDSubTerm& src);
	~CCNDSubTerm() { term = nullptr; }
public:
	std::shared_ptr<CCNDPrsTerm>	term;
	int			token;
};

CCNDSubTerm::CCNDSubTerm(const CCNDSubTerm& src)
{
	token = src.token;
	term = src.term;
}

// ////////////////////////////////////////////////////////////////////////////////////////
CCNDPrsExpression::CCNDPrsExpression(CPrsSymbol& sym) :CPrsNode(sym)
{
}
CCNDPrsExpression::CCNDPrsExpression(const CCNDPrsExpression& src) :CPrsNode(src.m_Symbol)
{
	m_Ls = src.m_Ls;
	m_Value = src.m_Value;
}
CCNDPrsExpression::~CCNDPrsExpression()
{
	Flush();
}
void CCNDPrsExpression::Flush()
{
	m_Ls.clear();
}
void CCNDPrsExpression::Generate(stackGntInfo& stinfo)
{
	SCNDMethod		sm;
	
	for (auto& it : m_Ls)
	{
		FVariant value;
		auto subterm = it;

		if (subterm->term)
		{
			subterm->term->Generate(stinfo);
			value = subterm->term->getValue();
		}

		switch (subterm->token)
		{
		case OROR:
			sm.Token = subterm->token;
			break;
		case noToken:
		{
			switch (sm.Token)
			{
			case OROR:
				sm.value = sm.value || value;
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

}
void CCNDPrsExpression::Parse()
{
	DEBUG_PARSE(CCNDPrsExpression)

	// term
	auto term = std::make_shared<CCNDPrsTerm>(m_Symbol);
	term->Parse();

	m_Ls.push_back(std::make_shared<CCNDSubTerm>(term));

	auto st = getSymbol();

	while (st.Token == OROR)
	{
		m_Ls.push_back(std::make_shared<CCNDSubTerm>(nullptr, st.Token));

		// term()
		getNewSymbol();
		term = std::make_shared<CCNDPrsTerm>(m_Symbol);
		term->Parse();

		m_Ls.push_back(std::make_shared<CCNDSubTerm>(term));
		st = getSymbol();
	}



}
/////////////////////////////////Trem///////////////////////////////////////////////////////
class CCNDSubFactor
{
public:
	CCNDSubFactor(std::shared_ptr<CCNDPrsFactor> p, int Token = noToken) { factor = p; token = Token; }
	CCNDSubFactor(const CCNDSubFactor& src)
	{
		token = src.token;
		factor = src.factor;
	}
	~CCNDSubFactor()
	{
		factor = nullptr;
	}
public:
	std::shared_ptr<CCNDPrsFactor> factor;
	int			token;
};

CCNDPrsTerm::CCNDPrsTerm(CPrsSymbol& sym) :CPrsNode(sym) {}
CCNDPrsTerm::CCNDPrsTerm(const CCNDPrsTerm& src) :CPrsNode(src.m_Symbol)
{
	m_Value = src.m_Value;
	m_Ls = src.m_Ls;
}
CCNDPrsTerm::~CCNDPrsTerm()
{
	Flush();
}

void CCNDPrsTerm::Flush()
{
	m_Ls.clear();
}
void CCNDPrsTerm::Generate(stackGntInfo& stinfo)
{
	SCNDMethod		sm;

	for (auto& it : m_Ls)
	{
		FVariant value;
		auto subfactor = it.get();

		if (subfactor->factor)
		{
			subfactor->factor->Generate(stinfo);
			value = subfactor->factor->getValue();
		}

		switch (subfactor->token)
		{
		case ANDAND:
			sm.Token = subfactor->token;
			break;
		case noToken:
		{
			switch (sm.Token)
			{
			case ANDAND:
				sm.value = sm.value && value;
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
}
void CCNDPrsTerm::Parse()
{
	DEBUG_PARSE(CCNDPrsTerm)

	// factor
	auto factor = std::make_shared<CCNDPrsFactor>(m_Symbol);
	factor->Parse();

	m_Ls.push_back(std::make_shared<CCNDSubFactor>(factor));

	SToken	st = getSymbol();

	while (st.Token == ANDAND)
	{
		int TokenMethod = st.Token;

		m_Ls.push_back(std::make_shared<CCNDSubFactor>(nullptr, TokenMethod));

		// factor
		st = getNewSymbol();
		factor = std::make_shared<CCNDPrsFactor>(m_Symbol);
		factor->Parse();

		m_Ls.push_back(std::make_shared<CCNDSubFactor>(factor));

		st = getSymbol();
	}
}

//////////////////////Factor////////////////////////////////////////////////////////////
CCNDPrsFactor::CCNDPrsFactor(CPrsSymbol& sym) :CPrsNode(sym)
{
	m_Token = 0;
}
CCNDPrsFactor::CCNDPrsFactor(const CCNDPrsFactor& src) :CPrsNode(src.m_Symbol)
{
	m_Value = src.m_Value;
	m_IdentName = src.m_IdentName;
	m_Token = src.m_Token;
	m_dot_next = src.m_dot_next;

	m_node = factory(src.m_node.get(), m_Token);
}
CCNDPrsFactor::~CCNDPrsFactor()
{
	Flush();
}

void CCNDPrsFactor::Flush()
{
	m_node = nullptr;
	m_dot_next = nullptr;
}

void CCNDPrsFactor::Parse()
{
	DEBUG_PARSE(CCNDPrsFactor)

		SToken	st = getSymbol();

	m_Token = st.Token;

	auto& localtbl = m_Symbol.getSymbolTable();
	bool bfunction_pointer = false;


	switch (m_Token)
	{
	case Ident:
	{
		auto nst = m_Symbol.getNewPeekSymbol();
		if (nst.Token == lParen)
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

		return;
	}
	break;
	case Quotation:
		m_Value.setS(st.Value);
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
		sm << L"CCNDPrsFactor::Parse() err token:" << m_Token << L" " << st.Value;
		THROW(sm.str());
	}
	break;
	}

	st = getSymbol();
	if (st.Token != Semicol)
		st = getNewSymbol();

}


void CCNDPrsFactor::ParselParen(SToken & st)
{
	getNewSymbol();
	m_node = factory(nullptr, m_Token);//new CCNDPrsExpression( symbol );
	m_node->Parse();
	auto st2 = getSymbol();
}

int CCNDPrsFactor::ParseIdent(SToken & st)
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
void CCNDPrsFactor::ParseNumber(SToken & st, __int64& outValue)
{
	outValue = _tstoi64(st.Value.c_str());
}
int CCNDPrsFactor::ParseIdentFunc(SToken & st)
{
	auto funcName = st.Value;

	auto nst = m_Symbol.getNewPeekSymbol(); //getNewSymbol();

	if (nst.Token == lParen)
	{
		getNewSymbol();
		m_node = factory(nullptr, m_Token); //new CCNDPrsParameters( symbol );
		m_node->Parse();
	}
	return 0;
}
int CCNDPrsFactor::ParseBuiltinFunc(SToken & st)
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

void CCNDPrsFactor::Generate(stackGntInfo & stinfo)
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

		//if (m_node)
		//{
		//	CCNDPrsVarFunction* varfunc = dynamic_cast<CCNDPrsVarFunction*>(m_node.get());

		//	if (varfunc)
		//	{
		//		FVariant temp(vSelf);
		//		varfunc->setgetValue(temp);
		//		varfunc->Generate(stinfo); // varÇÃä÷êî
		//		m_Value = varfunc->Return();
		//	}
		//	else if (dynamic_cast<CCNDPrsParameters*>(m_node.get()) && vSelf.vt == VT_UNKNOWN)
		//	{
		//		// a[0](); ÇÃ()ïîï™ÇÃé¿çs
		//		auto prmfunc = dynamic_cast<CCNDPrsParameters*>(m_node.get());
		//		prmfunc->Generate(stinfo);

		//		auto vfunc = dynamic_cast<IVARIANTAbstract*>(vSelf.punkVal);
		//		if (FVariantType::FUNCTIONPOINTER == vfunc->TypeId())
		//		{
		//			auto& ar = prmfunc->getParam();

		//			auto v = std::make_shared<FVariant[]>(ar.size());
		//			int i = 0;
		//			for (auto& it : ar)
		//			{
		//				v[i++] = it;
		//			}

		//			m_Value = vfunc->Invoke(L"NONAME", (VARIANT*)&v[0], (int)ar.size());
		//		}
		//	}
		//	else
		//		THROW(L"CCNDPrsFactor::Generate error");

		//}
		//else
		//{
		//	if (vSelf.vt == VTF_FUNC)
		//	{
		//		auto funcnm = m_IdentName;
		//		auto ifunc = new IVARIANTFunctionImp();
		//		auto func = m_Symbol.CreateFunc(funcnm);
		//		func->Generate(stinfo);
		//		ifunc->SetItem(func);

		//		//::VariantClear(&m_Value);
		//		m_Value.clear_init();

		//		m_Value.vt = VT_UNKNOWN;
		//		m_Value.punkVal = ifunc;
		//	}
		//	else
		//		m_Value = vSelf;
		//}


		//::VariantClear(&vSelf);
	}
	break;

	case Float:
	case Quotation:
	case Number:
		//TRACE( "%s\n", (LPCTSTR)m_IdentName );
		break;
	case lParen:
		if (m_node)
		{
			m_node->Generate(stinfo);
			_ASSERT(dynamic_cast<CCNDPrsExpression*>(m_node.get()));

			m_Value = dynamic_cast<CCNDPrsExpression*>(m_node.get())->getValue();
		}
		break;
	case IdentFunc:
	{
		//if (m_node)
		//{
		//	auto param = std::make_unique<CCNDPrsParameters>(*((CCNDPrsParameters*)m_node.get()));
		//	auto func = m_Symbol.CreateFunc(m_IdentName);

		//	param->Generate(stinfo);
		//	func->SetParameters(*param);
		//	func->Generate(stinfo);
		//	m_Value = func->getValue();


		//	func = nullptr;
		//}
	}
	break;
	


	case lSquare:
	{
		// generate FVariantArray, 
		std::vector<FVariant> data;

		auto it = m_node;
		while (true)
		{
			it->Generate(stinfo);
			CCNDPrsExpression* exp = (CCNDPrsExpression*)it.get();
			data.push_back(exp->getValue());

			if (exp->next_)
				it = exp->next_;
			else
				break;
		}
		m_Value.setAr(data);
	}
	break;
	
	case lBracket:
	{
		// generate FVariantMap
		std::map<wstring, FVariant> map;

		auto it = m_node;
		while (true)
		{
			CCNDPrsExpression* exp = (CCNDPrsExpression*)it.get();

			if (exp)
			{
				FVariant key;
				key = exp->getValue(); //>m_Value;

				if (key.vt = VT_BSTR && key.bstrVal)
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

	break;
	}
}

std::shared_ptr<CPrsNode> CCNDPrsFactor::factory(CPrsNode * src, int type, LPCTSTR funcName)
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
			node = std::make_shared<CCNDPrsExpression>(*(CCNDPrsExpression*)src);
		else
			node = std::make_shared<CCNDPrsExpression>(m_Symbol);
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
