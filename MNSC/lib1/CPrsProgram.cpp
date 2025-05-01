#include "pch.h"
#include "CSymbolTable.h"
#include "CPrsNode.h"

static int glinenum_debug = 0;
///////////////////////////////////////////////////////////////////////////////////
CPrsProgram::CPrsProgram(CPrsSymbol& sym) :CPrsNode(sym)
{

}
void CPrsProgram::Generate(stackGntInfo& stinfo)
{
	blk_->Generate(stinfo);
}
void CPrsProgram::Generate(stackGntInfo& stinfo, LPCWSTR funcnm, VARIANT* prms, int prmscnt)
{
	blk_->Generate(stinfo,funcnm, prms, prmscnt);
}
void CPrsProgram::Flush()
{
	if (blk_)
		blk_->Flush();
	blk_ = nullptr;
}

void CPrsProgram::Parse()
{
	DEBUG_PARSE(CPrsProgram)

	glinenum_debug = 0;

	// Pre-register functions
	auto ar = m_Symbol.getFuncNames();
	for (auto& nm : ar)
		m_Symbol.RegistMasterFunc(nm, true);

	// Regist Builtin function 
	CPrsBuiltinFunction::Initialize(m_Symbol.getGlobalSymbolTable());

	// start!
	blk_ = std::make_shared<CPrsBlock>(m_Symbol);

	getNewSymbol();
	blk_->Parse();
}
void CPrsProgram::SetPreGlobalVariant(LPCWSTR ident, VARIANT val)
{
	FVariant f(val);

	if (ident)
		m_Symbol.getGlobalSymbolTable().setAt(ident,f);

}
FVariant CPrsProgram::Return() const
{ 
	return blk_->Return(); 
}
///////////////////////////////////////////////////////////////////////////////////
CPrsBlock::CPrsBlock(CPrsSymbol& sym) :CPrsNode(sym), top_(nullptr)
{
	
}
FVariant CPrsBlock::Return() const
{
	return return_;
}
void CPrsBlock::Parse()
{
	DEBUG_PARSE(CPrsBlock)
	CPrsDecaration	decaration(m_Symbol);
	decaration.Parse();
}
void CPrsBlock::Generate(stackGntInfo& stinfo)
{
	
}
void CPrsBlock::Generate(stackGntInfo& stinfo, LPCWSTR funcnm, VARIANT* prms, int prmscnt)
{
	std::shared_ptr<CPrsFunction> func = m_Symbol.GetFunc(funcnm);
	
	if ( func == nullptr )
	{
		std::wstringstream sm;
		sm << L"not found function: " << funcnm;				
		THROW(sm.str());
	}

	func->SetParameters(prms, prmscnt);
	func->Generate(stinfo);


	return_ = func->getValue();

	top_ = func;
}
void CPrsBlock::Flush()
{
	if ( top_)
		top_->Flush();
	top_ = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////
CPrsBody::CPrsBody(CPrsSymbol& sym) :CPrsNode(sym)
{
}
CPrsBody::CPrsBody(const CPrsBody& src) :CPrsNode(src.m_Symbol)
{
	m_statementlist = std::make_shared<CPrsStatmentList>(*(src.m_statementlist));
	m_ReturnValue = src.m_ReturnValue;
}
CPrsBody::~CPrsBody()
{
	Flush();
}
void CPrsBody::Flush()
{
	m_statementlist = nullptr;
}
void CPrsBody::Generate(stackGntInfo& stinfo)
{
	if (m_statementlist)
	{

		stinfo.top().m_stackInfo.push(CPrsGntInfo::out_loop);

		m_statementlist->Generate(stinfo);

		stinfo.top().m_stackInfo.pop();

		// ä÷êîÇÃreturnílÇÃâÒé˚
		CSymbolTable& tbl = m_Symbol.getSymbolTable();

		//m_ReturnValue = stinfo.top().m_ReturnValue;

		FVariant xx = stinfo.top().m_ReturnValue;
		m_ReturnValue = xx;
	}

}
void CPrsBody::Parse()
{
	DEBUG_PARSE(CPrsBody)
	try
	{

		SToken	st = getSymbol();

		switch (st.Token)
		{
			case beginSym:
				m_statementlist = std::make_shared<CPrsStatmentList>(m_Symbol, ENDSYMBOL_END);
			break;
			default:
				THROW( L"'BEGIN' expected");		
		}

		if (m_statementlist)
		{
			getNewSymbol();
			m_statementlist->Parse();
		}

		st = getSymbol();

		if (st.Token != endSym)
			THROW(L"'END' expected");

		getNewSymbol();
	}
	catch(std::runtime_error& er)
	{
		Flush();
		throw er;
	}

}


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


	try{

		SToken st = getSymbol();

		if ( st.Token == OROR || st.Token == ANDAND )
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
	catch(std::runtime_error& er)
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
}
CPrsStatment::CPrsStatment(const CPrsStatment& src) :CPrsNode(src.m_Symbol)
{
	m_Nodetype = src.m_Nodetype;
	m_Subndtype = src.m_Subndtype;
	m_LineNumber = src.m_LineNumber;

	m_node = factoryParse(m_Nodetype, src.m_node.get(), m_Subndtype);
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
	catch(wstring err)
	{
		std::wstringstream sm;
		sm << L"\r\nruntime error linenumber:" << m_LineNumber ;
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
	const SToken&	st = getSymbol();
	CPrsNode* node = nullptr;

	if ( st.Token == rParen )
	{
		getNewSymbol();
		return;
	}


	m_LineNumber = m_Symbol.getLineNumber();
	try
	{
		m_Nodetype = st.Token;

		SToken stahead = m_Symbol.getNewSymbolReal();
		m_Subndtype = stahead.Token;

		m_node = factoryParse(m_Nodetype, nullptr, m_Subndtype);
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
		throw std::runtime_error( w2a(msg));
	}
}
std::shared_ptr<CPrsNode> CPrsStatment::factoryParse(int nodetype, CPrsNode* src, int subnodetype)
{
	CPrsNode* node = nullptr;

	switch (nodetype)
	{
		case Ident:
		{
			if (src)
			{
				if (subnodetype == lSquare)
					node = new CPrsAssignSquare(*(CPrsAssignSquare*)src);
				else if (subnodetype == Plusplus || subnodetype == Minusminus || subnodetype == PlusEqual || subnodetype == MinusEqual)
					node = new CPrsAssignPlusEqual(*(CPrsAssignPlusEqual*)src);
				else
					node = new CPrsAssign(*(CPrsAssign*)src);
			}
			else
			{
				if (subnodetype == lSquare)
				{
					node = new CPrsAssignSquare(m_Symbol);
					node->Parse();
				}				
				else if (subnodetype == Plusplus || subnodetype == Minusminus || subnodetype == PlusEqual || subnodetype == MinusEqual)
				{
					node = new CPrsAssignPlusEqual(m_Symbol);
					node->Parse();
				}
				else
				{
					CPrsAssign* assign_node = new CPrsAssign(m_Symbol);

					assign_node->HParse();

					return std::shared_ptr<CPrsNode>(assign_node);
				}
			}
		}
		break;
		case returnSym:
			if (src)
				node = new CPrsReturn(*(CPrsReturn*)src);
			else
			{
				node = new CPrsReturn(m_Symbol);
				node->Parse();
			}
		break;

		case ifSym:
			if (src)
				node = new CPrsIf(*(CPrsIf*)src);
			else
			{
				node = new CPrsIf(m_Symbol);
				node->Parse();
			}
		break;
		case forSym:
			if (src)
				node = new CPrsFor(*(CPrsFor*)src);
			else
			{
				node = new CPrsFor(m_Symbol);
				node->Parse();
			}
		break;
		case breakSym:
			if (src)
				node = new CPrsBreak(*((CPrsBreak*)src));
			else
			{
				node = new CPrsBreak(m_Symbol);
				node->Parse();
			}
		break;

		case whileSym:
			if (src)
				node = new CPrsWhile(*(CPrsWhile*)src);
			else
			{
				node = new CPrsWhile(m_Symbol);
				node->Parse();
			}
		break;

		case doSym:
			if (src)
				node = new CPrsDo(*(CPrsDo*)src);
			else
			{
				node = new CPrsDo(m_Symbol);
				node->Parse();
			}
		break;
		case printSym:
			if (src)
				node = new CPrsPrint(*(CPrsPrint*)src);
			else
			{
				node = new CPrsPrint(m_Symbol);
				node->Parse();
			}
		break;

		default:
			break;
	}
	return std::shared_ptr<CPrsNode>(node);
}


