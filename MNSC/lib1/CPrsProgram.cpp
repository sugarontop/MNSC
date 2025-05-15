#include "pch.h"
#include "CSymbolTable.h"
#include "CPrsNode.h"

extern int glinenum_debug;

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


