#include "pch.h"
#include "CSymbolTable.h"
#include "CPrsNode.h"

int glinenum_debug = 0;

///////////////////////////////////////////////////////////////////////////////////
CPrsStatmentList::CPrsStatmentList(CPrsSymbol& sym, int endsymbol) :CPrsNode(sym), m_Endsymbol(endsymbol)
{

}
CPrsStatmentList::CPrsStatmentList(const CPrsStatmentList& src) :CPrsNode(src.m_Symbol), m_Endsymbol(src.m_Endsymbol)
{
	m_ls = src.m_ls;
}
CPrsStatmentList::~CPrsStatmentList()
{
	Flush();
}
void CPrsStatmentList::Flush()
{
	m_ls.clear();
}

void CPrsStatmentList::Generate(stackGntInfo& stinfo)
{
	for (auto& it : m_ls)
	{
		it->HGenerate(stinfo);

		if (stinfo.top().m_bReturn)
			break;
		if (stinfo.top().m_stackInfo.top() == CPrsGntInfo::outof_loop)
			break;
	}
}
void CPrsStatmentList::Parse()
{
	DEBUG_PARSE(CPrsStatmentList)
	

	try 
	{
		SToken st = getSymbol();

		if (st.Token == OROR || st.Token == ANDAND)
			THROW(L"token OROR or ANDAND");


		// loop
		for (;;)
		{
			auto statement = std::make_shared<CPrsStatment>(m_Symbol);
			m_ls.push_back(statement);

			statement->HParse();

			st = getSymbol();

			switch (st.Token)
			{
			case endSym:
				if (m_Endsymbol & ENDSYMBOL_END) return;
				break;
			case nextSym:
				if (m_Endsymbol & ENDSYMBOL_ENDNEXT) return;
				break;
			case elseSym:
				if (m_Endsymbol & ENDSYMBOL_ELSE) return;
				break;
			case endifSym:
				if (m_Endsymbol & ENDSYMBOL_ENDIF) return;
				break;
			case whileSym:
				if (m_Endsymbol & ENDSYMBOL_WHILEEND) return;
				break;
			case elsifSym:
				if (m_Endsymbol & ENDSYMBOL_ELSIF) return;
				break;
			}
		}
	}
	catch (std::runtime_error& er)
	{
		Flush();
		throw er;
	}
}


///////////////////////////////////////////////////////////////////////////////////
CPrsStatment::CPrsStatment(CPrsSymbol& sym) :CPrsNode(sym)
{
	m_Nodetype = 0;
	m_Subndtype = 0;
	m_LineNumber = 0;
	m_bconst = false;
}
CPrsStatment::CPrsStatment(const CPrsStatment& src) :CPrsNode(src.m_Symbol)
{
	m_Nodetype = src.m_Nodetype;
	m_Subndtype = src.m_Subndtype;
	m_LineNumber = src.m_LineNumber;
	m_bconst = src.m_bconst;

	m_node = factoryParse(m_Nodetype, src.m_node.get(), m_Subndtype, src.m_bconst);
}

CPrsStatment::~CPrsStatment()
{
	Flush();
}
void CPrsStatment::Flush()
{
	m_node = nullptr;
	m_Nodetype = 0;
	m_Subndtype = 0;
}
void CPrsStatment::HGenerate(stackGntInfo& stinfo)
{
	try
	{
		if (m_node)
			m_node->Generate(stinfo);
	}
	catch (wstring err)
	{
		std::wstringstream sm;
		sm << L"\r\nruntime error linenumber:" << m_LineNumber;
		err += sm.str();
		throw err;
	}

}


void CPrsStatment::HParse()
{
	DEBUG_PARSE(CPrsStatment)


		if (glinenum_debug++ > 1000)
		{
			THROW(L"limiter over 1000 lines");
		}
	SToken	st = getSymbol();

	if (st.Token == rParen)
	{
		getNewSymbol();
		return;
	}

	m_bconst = false;
	m_LineNumber = m_Symbol.getLineNumber();
	try
	{
		if (st.Token == constSym)
		{
			st = getNewSymbol();
			m_bconst = true;
		}



		m_Nodetype = st.Token;

		SToken stahead = m_Symbol.getNewSymbolReal();
		m_Subndtype = stahead.Token;

		m_node = factoryParse(m_Nodetype, nullptr, m_Subndtype, m_bconst);
		if (m_node)
		{
			auto& st2 = getSymbol();
			if (st2.Token == Semicol)
			{
				THROW(L"';' not requried");		// Ç±ÇÃíiäKÇ≈ÇÕ';'ÇÃéüÇÃtokenÇ™ì™èoÇµÇ≥ÇÍÇƒÇ¢ÇÈÇÕÇ∏			
			}
		}


	}
	catch (wstring msg)
	{
		throw std::runtime_error(w2a(msg));
	}
}

std::shared_ptr<CPrsNode> CPrsStatment::factoryParse(int nodetype, CPrsNode* src, int subnodetype, bool bconst)
{
	_ASSERT(src == nullptr);

	switch (nodetype)
	{
	case Ident:
	{
		if (subnodetype == lSquare)
		{
			auto nd = std::make_shared <CPrsAssignSquare>(m_Symbol);
			nd->Parse();
			return nd;
		}
		else if (subnodetype == Plusplus || subnodetype == Minusminus || subnodetype == PlusEqual || subnodetype == MinusEqual)
		{
			auto nd = std::make_shared <CPrsAssignPlusEqual>(m_Symbol);
			nd->Parse();
			return nd;
		}
		else
		{
			auto nd = std::make_shared <CPrsAssign>(m_Symbol);

			nd->lock(bconst);
			nd->HParse();
			return nd;
		}

	}
	break;
	case returnSym:
	{
		auto nd = std::make_shared <CPrsReturn>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;

	case ifSym:
	{
		auto nd = std::make_shared <CPrsIf>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;
	case forSym:
	{
		auto nd = std::make_shared <CPrsFor>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;
	case breakSym:
	{
		auto nd = std::make_shared <CPrsBreak>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;

	case whileSym:
	{
		auto nd = std::make_shared <CPrsWhile>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;

	case doSym:
	{
		auto nd = std::make_shared <CPrsDo>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;
	case printSym:
	{
		auto nd = std::make_shared <CPrsPrint>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;
	case AssertSym:
	{
		auto nd = std::make_shared<CPrsAssert>(m_Symbol);
		nd->Parse();
		return nd;
	}
	break;
	default:
		THROW(L"factoryParse err");
	}
}


//std::shared_ptr<CPrsNode> CPrsStatment::factoryParse(int nodetype, CPrsNode* src, int subnodetype, bool bconst)
//{
//	CPrsNode* node = nullptr;
//
//_ASSERT(src==nullptr);
//
//	switch (nodetype)
//	{
//		case Ident:
//		{
//			if (src)
//			{
//				if (subnodetype == lSquare)
//					node = new CPrsAssignSquare(*(CPrsAssignSquare*)src);
//				else if (subnodetype == Plusplus || subnodetype == Minusminus || subnodetype == PlusEqual || subnodetype == MinusEqual)
//					node = new CPrsAssignPlusEqual(*(CPrsAssignPlusEqual*)src);
//				else
//					node = new CPrsAssign(*(CPrsAssign*)src);
//			}
//			else
//			{
//				if (subnodetype == lSquare)
//				{
//					node = new CPrsAssignSquare(m_Symbol);
//					node->Parse();
//				}				
//				else if (subnodetype == Plusplus || subnodetype == Minusminus || subnodetype == PlusEqual || subnodetype == MinusEqual)
//				{
//					node = new CPrsAssignPlusEqual(m_Symbol);
//					node->Parse();
//				}
//				else
//				{
//					CPrsAssign* assign_node = new CPrsAssign(m_Symbol);
//
//					assign_node->lock(bconst);
//
//					assign_node->HParse();
//
//					return std::shared_ptr<CPrsNode>(assign_node);
//				}
//			}
//		}
//		break;
//		case returnSym:
//			if (src)
//				node = new CPrsReturn(*(CPrsReturn*)src);
//			else
//			{
//				node = new CPrsReturn(m_Symbol);
//				node->Parse();
//			}
//		break;
//
//		case ifSym:
//			if (src)
//				node = new CPrsIf(*(CPrsIf*)src);
//			else
//			{
//				node = new CPrsIf(m_Symbol);
//				node->Parse();
//			}
//		break;
//		case forSym:
//			if (src)
//				node = new CPrsFor(*(CPrsFor*)src);
//			else
//			{
//				node = new CPrsFor(m_Symbol);
//				node->Parse();
//			}
//		break;
//		case breakSym:
//			if (src)
//				node = new CPrsBreak(*((CPrsBreak*)src));
//			else
//			{
//				node = new CPrsBreak(m_Symbol);
//				node->Parse();
//			}
//		break;
//
//		case whileSym:
//			if (src)
//				node = new CPrsWhile(*(CPrsWhile*)src);
//			else
//			{
//				node = new CPrsWhile(m_Symbol);
//				node->Parse();
//			}
//		break;
//
//		case doSym:
//			if (src)
//				node = new CPrsDo(*(CPrsDo*)src);
//			else
//			{
//				node = new CPrsDo(m_Symbol);
//				node->Parse();
//			}
//		break;
//		case printSym:
//			if (src)
//				node = new CPrsPrint(*(CPrsPrint*)src);
//			else
//			{
//				node = new CPrsPrint(m_Symbol);
//				node->Parse();
//			}
//		break;
//		case AssertSym:
//			if (src)
//				node = new CPrsAssert(*(CPrsAssert*)src);
//			else
//			{
//				node = new CPrsAssert(m_Symbol);
//				node->Parse();
//			}
//		break;
//		default:
//			break;
//	}
//	return std::shared_ptr<CPrsNode>(node);
//}


