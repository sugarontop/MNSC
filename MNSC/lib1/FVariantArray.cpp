#include "pch.h"
#include "CPrsNode.h"
#include "FVariantArray.h"
#include "FVariant.h"

#define IUNKNOWN_ITERFACE					\
if (riid == IID_IUnknown){					\
*ppv = static_cast<IUnknown*>(this);		\
	}										\
else {										\
	*ppv = nullptr;							\
	return E_NOINTERFACE;					\
	}										\
	AddRef();								\
	return S_OK								\


void IVARIANTArrayImp::Add(VARIANT& v)
{
	VARIANT x;
	::VariantInit(&x);
	if (S_OK!=::VariantCopy(&x, &v))
		THROW(L"VariantCopy err 1");
	ar.push_back(x);
}
bool IVARIANTArrayImp::Get(int idx, VARIANT* dst)
{
	::VariantClear(dst);

	if (idx < (int)ar.size())
	{
		if (S_OK != ::VariantCopy(dst, &ar[idx]))		
			THROW(L"VariantCopy err 2");
	}
	else
		THROW(L"array size err");
	return true;
}
void IVARIANTArrayImp::Clear()
{
	for (auto& v : ar)
		::VariantClear(&v);
	ar.clear();
}
void IVARIANTArrayImp::Set(int idx, VARIANT& v)
{
	if (idx < (int)ar.size())
	{
		VARIANT old = ar[idx];
		::VariantClear(&old);
		::VariantInit(&ar[idx]);
		if ( S_OK != ::VariantCopy(&ar[idx], &v))
			THROW(L"VariantCopy err 3");
	}
	else
		THROW(L"array size err");
}
HRESULT __stdcall IVARIANTArrayImp::QueryInterface(REFIID riid, void** ppv) { IUNKNOWN_ITERFACE;}
HRESULT __stdcall IVARIANTMapImp::QueryInterface(REFIID riid, void** ppv) { IUNKNOWN_ITERFACE;}
HRESULT __stdcall IVARIANTFunctionImp::QueryInterface(REFIID riid, void** ppv) { IUNKNOWN_ITERFACE;}
HRESULT __stdcall IVARIANTClassImp::QueryInterface(REFIID riid, void** ppv) { IUNKNOWN_ITERFACE; }

// Array ---------------------------------------------------------------------------------------------------

VARIANT IVARIANTArrayImp::Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt)
{
	std::wstring funcnm = cfuncnm;
	VARIANT x;
	::VariantInit(&x);

	if (funcnm == L"length" || funcnm == L"count")
	{
		FVariant cnt( (int)Count());
		x = cnt.ToVARIANT();
	}
	else if (funcnm == L"get" && vcnt > 0)
	{
		int idx = (int)v[0].llVal;
		Get(idx, &x);
	}
	else if (funcnm == L"add" && vcnt > 0)
	{
		Add(v[0]);
	}
	else
	{
		std::wstringstream sm;
		sm << L"not implemnt: " << funcnm;
		THROW(sm.str());
	}

	return x;
}
// Map ---------------------------------------------------------------------------------------------------
IVARIANTMapImp::IVARIANTMapImp()
{

}
VARIANT IVARIANTMapImp::Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt)
{
	std::wstring funcnm = cfuncnm;
	VARIANT x;
	::VariantInit(&x);

	if (funcnm == L"length" || funcnm == L"count")
	{
		FVariant cnt((int)Count());
		x = cnt.ToVARIANT();
	}
	else if (funcnm == L"get" && vcnt > 0 && v[0].vt == VT_BSTR)
	{
		wstring key = v[0].bstrVal;
		if (!GetItem(key.c_str(), &x))
		{
			THROW(L"map err");
		}
	}
	else if (funcnm == L"keys")
	{
		std::vector<FVariant> ar;
		for(auto& it :this->map)
			ar.push_back(FVariant(it.first.c_str()));

		FVariant ret;
		ret.setAr(ar);
		x = ret.ToVARIANT();
	}
	else
	{
		std::wstringstream sm;
		sm << L"not implemnt: " << funcnm;
		THROW(sm.str());
	}
	return x;
}

// Function ---------------------------------------------------------------------------------------------------



IVARIANTFunctionImp::IVARIANTFunctionImp()
{	
	
}
VARIANT IVARIANTFunctionImp::Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt)
{
	std::wstring funcnm = cfuncnm;
	if ( funcnm == L"NONAME")
	{
		stackGntInfo temp;

		_ASSERT(func_);

		func_->SetParameters(v,vcnt);		
		func_->Generate(temp);


		VARIANT ret = func_->getValue().ToVARIANT();
		
		func_->ClearParameters();

		

		return ret;
	}

	VARIANT x;
	::VariantInit(&x);
	return x;
}
void IVARIANTFunctionImp::Clear()
{ 
	func_ = nullptr; 
}

// Class ---------------------------------------------------------------------------------------------------
IVARIANTClassImp::IVARIANTClassImp()
{
	
}
VARIANT IVARIANTClassImp::Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt)
{
	std::wstring funcnm = cfuncnm;
	VARIANT x;
	::VariantInit(&x);

	if (funcnm == L"length" || funcnm == L"count")
	{
		FVariant cnt((int)Count());
		x = cnt.ToVARIANT();
	}
	/*else if (funcnm == L"get" && vcnt > 0 && v[0].vt == VT_BSTR)
	{
		wstring key = v[0].bstrVal;
		if (!GetItem(key.c_str(), &x))
		{
			THROW(L"map err");
		}
	}*/
	else if (funcnm == L"keys")
	{
		std::vector<FVariant> ar;
		for (auto& it : this->map)
			ar.push_back(FVariant(it.first.c_str()));

		FVariant ret;
		ret.setAr(ar);
		x = ret.ToVARIANT();
	}
	else
	{
		std::wstringstream sm;
		sm << L"not implemnt: " << funcnm;
		THROW(sm.str());
	}
	return x;
}