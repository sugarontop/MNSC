#include "pch.h"
#include "CPrsNode.h"
#include "FVariantArray.h"

///////////////////////////////////////////////////////////////////////////////////
CPrsIdentArray::CPrsIdentArray(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsIdentArray::CPrsIdentArray(const CPrsIdentArray& src) :CPrsNode(src.m_Symbol)
{
	Return_ = src.Return_;
	expression_ = src.expression_;
	next_ = src.next_;
}
CPrsIdentArray::~CPrsIdentArray()
{
	Flush();
}
void CPrsIdentArray::Flush()
{
	expression_ = nullptr;
	next_ = nullptr;
}
void CPrsIdentArray::Parse()
{
	auto stpre = m_Symbol.getPreSymbol();
	auto st = getSymbol();
	if ( st.Token == lSquare )
	{
		st = getNewSymbol();
		// key 
		expression_ = std::make_shared<CPrsExpression>(m_Symbol);
		expression_->Parse();
	}

	st = getSymbol(); // rSquare‚ÌŒã‚ë‚ÍFX‚ ‚é

	if (st.Token == rSquare)
	{
		st = getNewSymbol();
	}
}
void CPrsIdentArray::Generate(stackGntInfo& stinfo)
{
	FVariant PrValue = Return_;

	Return_.clear();

	expression_->Generate(stinfo);

	if (PrValue.vt == VT_UNKNOWN)
	{
		VARIANT v;
		::VariantInit(&v);

		auto ar = dynamic_cast<IVARIANTArray*>(PrValue.punkVal);
		if ( ar )
		{
			auto idx = (int)expression_->getValue().getN();
			if (ar->Get(idx, &v))
				Return_ = v;
			else
				THROW(L"CPrsIdentArray err1");
		}
		else
		{
			auto map = dynamic_cast<IVARIANTMap*>(PrValue.punkVal);
			_ASSERT(map);

			std::wstring key = expression_->getValue().getS();
			if (map->GetItem(key, &v))
				Return_ = v;
			else
				THROW(L"CPrsIdentArray err2");
		}
	}
	else if ( Return_.vt == 0)
		Return_ = expression_->getValue();
	
	if ( next_ )
	{
		auto nnext = dynamic_cast<CPrsIdentArray*>(next_.get());
		
		nnext->setgetValue(Return_);
		nnext->Generate(stinfo);
		Return_ = nnext->Value();

	}
}
void CPrsIdentArray::setgetValue(const FVariant& var)
{
	Return_ = var;
}

///------------------------------------------
CPrsIdentFunctionPointer::CPrsIdentFunctionPointer(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsIdentFunctionPointer::CPrsIdentFunctionPointer(const CPrsIdentFunctionPointer& src) :CPrsNode(src.m_Symbol)
{
	Return_ = src.Return_;
}
CPrsIdentFunctionPointer::~CPrsIdentFunctionPointer()
{
	Flush();
}
void CPrsIdentFunctionPointer::Flush()
{

}
void CPrsIdentFunctionPointer::Parse()
{
	auto stpre = m_Symbol.getPreSymbol();
	auto st = getSymbol();
	funcnm_ = stpre.Value;

}
void CPrsIdentFunctionPointer::Generate(stackGntInfo& stinfo)
{
	Return_.clear();

	auto x = new IVARIANTFunctionImp(); 

	auto func = m_Symbol.CreateFunc(funcnm_);
	x->SetItem(func);

	Return_.vt = VT_UNKNOWN;
	Return_.punkVal = x;
}