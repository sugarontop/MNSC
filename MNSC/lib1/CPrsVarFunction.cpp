#include "pch.h"
#include "CPrsNode.h"
#include "FVariant.h"
#include "mnsc.h"

///////////////////////////////////////////////////////////////////////////////////
CPrsVarFunction::CPrsVarFunction( CPrsSymbol& sym ):CPrsNode( sym )
{
	
}
CPrsVarFunction::CPrsVarFunction( const CPrsVarFunction& src ):CPrsNode( src.m_Symbol )
{
	m_value = src.m_value;
	DotFuncName_ = src.DotFuncName_;
	
	m_parameter = src.m_parameter;
	next_ = src.next_;
	m_expression = src.m_expression;
	Return_ = src.Return_;
}
CPrsVarFunction::~CPrsVarFunction()
{
	Flush();

}
void CPrsVarFunction::Parse()
{
	SToken			st = getSymbol();
	CSymbolTable&	symtbl = m_Symbol.getSymbolTable();

	wstring FuncName = st.Value;
	int token = st.Token;

	if ( token != lSquare && FuncName != L"") // ä÷êîÉ|ÉCÉìÉ^ÇÃèÍçáFuncNameÇÕÇ»Ç¢
		st = getNewSymbol();

	if ( !m_parameter )
			m_parameter = std::make_shared<CPrsParameters>( m_Symbol );

	if ( st.Token == lParen )
		m_parameter->Parse();
	
	DotFuncName_ = FuncName;


	st = getSymbol();
	if ( st.Token != Semicol && st.Token == rParen )
		st = getNewSymbol(); // ';'
}
void CPrsVarFunction::Parse2()
{
	SToken			st = getSymbol();
	CSymbolTable& symtbl = m_Symbol.getSymbolTable();

	int token = st.Token;
		
	m_expression = std::make_shared<CPrsExpression>(m_Symbol);
	m_expression->Parse();

	st = getNewSymbol();

	
	if (st.Token == Dot)
	{
		getNewSymbol();
		auto next = std::make_shared<CPrsVarFunction>(m_Symbol);
		next->Parse();
		next_ = next;
	}
	else if (st.Token == lSquare)
	{
		getNewSymbol();
		auto next = std::make_shared<CPrsVarFunction>(m_Symbol);
		next->Parse();
		next_ = next;
	}
}

void CPrsVarFunction::Generate2(stackGntInfo& stinfo)
{
	m_expression->Generate(stinfo);
	auto idx = m_expression->getValue();
	VARIANT v[10] = {};

	if (m_value.vt == VT_UNKNOWN)
	{

	}
	else if (m_value.vt == VT_BSTR)
	{
		bool bUpdate = false;
		v[0] = m_value.ToVARIANT();
		v[1] = idx.ToVARIANT();

		DotFuncName_ = L"[]";

		Return_ = VarInvoke(DotFuncName_, &bUpdate, v, 2);
		if (next_)
		{
			next_->setgetValue(Return_);

			next_->Generate(stinfo);

			Return_ = next_->Return();
		}
	}
}
const FVariant& CPrsVarFunction::Return() 
{	
	return Return_;
}
void CPrsVarFunction::setgetValue( const FVariant& var )
{
	m_value = var;
}
void CPrsVarFunction::Flush()
{
	m_parameter = nullptr;
	m_expression = nullptr;

}
void CPrsVarFunction::Generate( stackGntInfo& stinfo )
{	
	if (m_expression)
	{
		Generate2(stinfo);
		return;
	}


	m_parameter->Generate( stinfo );
	
	auto& ls = m_parameter->getParam();
	int param_count = (int)ls.size();
	_ASSERT(param_count < 10);
	VARIANT v[10] = {};

	if (m_value.vt == VT_UNKNOWN )
	{
		// Variant fcuntion
		IVARIANTAbstract* p = dynamic_cast<IVARIANTAbstract*>(m_value.punkVal);
		
		auto nm = (DotFuncName_.empty() ? L"NONAME" : DotFuncName_);

		for (int i = 0; i < param_count; i++)
		{									
			::VariantInit(&v[i]);
			v[i] = ls[i].ToVARIANT();
		}

		Return_ = p->Invoke(nm, v, param_count);

		for (int i = 0; i < param_count; i++)
		{
			::VariantClear(&v[i]);
		}
	}
	else
	{
		// Ident function

		_ASSERT(m_value.vt != VT_EMPTY);

		bool bUpdate = false;
		::VariantInit(&v[0]);
		v[0] = m_value.ToVARIANT();

		for (int i = 1; i < param_count+1; i++)
		{
			::VariantInit(&v[i]);
			v[i] = ls[i-1].ToVARIANT();
		}
		
		Return_ = VarInvoke(DotFuncName_, &bUpdate, v, param_count + 1);

		VariantClear(&v[0]);
		for (int i = 1; i < param_count + 1; i++)
		{
			::VariantClear(&v[i]);
		}

		if ( next_ )
		{
			next_->setgetValue(Return_);

			next_->Generate(stinfo);

			Return_ = next_->Return();
		}

	}

}

struct Xfunc
{
	Xfunc()
	{
		VarFunctionInit(func_map_);
	}
	std::map<std::wstring, VARIANTFUNC> func_map_;
};

VARIANT CPrsVarFunction::VarInvoke(wstring funcnm, bool* bUpdate, VARIANT* v, int vcnt)
{
	_ASSERT(vcnt > 0);

	static Xfunc x;

	VARIANT vempty;
	::VariantInit(&vempty);

	auto f = x.func_map_[funcnm];

	_ASSERT(vcnt < 5 );

	if ( f != nullptr )
	{
		if ( vcnt == 1 )
			return f(v[0], vempty, vempty, vempty);
		else if (vcnt == 2)
			return f(v[0], v[1], vempty, vempty);
		else if (vcnt == 3)
			return f(v[0], v[1], v[2], vempty);
		else if (vcnt == 4)
			return f(v[0], v[1], v[2], v[3]);
	}

	return FVariant(L"VarInvoke err").ToVARIANT();
}

