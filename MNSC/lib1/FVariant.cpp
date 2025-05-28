#include "pch.h"
#include "FVariant.h"
#include <strsafe.h>
#include "FVariantArray.h"

FVariant::FVariant(LPCWSTR str)
{
	init();
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(str);
}
FVariant::FVariant(__int64 x)
{
	init();
	vt = VT_I8;
	llVal = x;
}
FVariant::FVariant(int x)
{
	init();
	vt = VT_I8;
	llVal = x;
}
FVariant::FVariant(double x)
{
	init();
	vt = VT_R8;
	dblVal = x;
}
FVariant::FVariant(bool x)
{
	init();
	vt = VT_BOOL;
	boolVal = (x?-1:0);
}


void FVariant::init()
{ 
	memset(this,0,sizeof(VARIANT));
	::VariantInit(this);
	lock_ = false;
}
void FVariant::clear_init()
{
	if ( vt != 0 )
		clear();
	::VariantInit(this);
}
void FVariant::clear()
{			
	::VariantClear(this);
}
void FVariant::detach()
{
	vt = VT_EMPTY; // stop VariantClear
}

FVariant::FVariant(const FVariant& var)
{
	init();
	InnerCopy((VARIANT *)&var);

	lock_ = var.lock_;
}

void FVariant::InnerCopy(const VARIANT* src)
{
	if (src->vt >= VT_RESERVED)
		vt = src->vt;
	else if (src->vt > 0 && S_OK != VariantCopy(this, src))
		throw std::wstring(L"FVariant::InnerCopy err");
}
FVariant::FVariant(VARIANT v)
{
	init();

	InnerCopy(&v);
}

VARIANT FVariant::ToVARIANT() const
{
	VARIANT dst;
	::VariantInit(&dst);
	ToCopy(&dst);
	return dst;
}

void FVariant::ToCopy(VARIANT* dst) const
{
	// dst‚ÍVariantInitÏ‚Ý‚Æ‚·‚é
	_ASSERT(dst->vt == 0);
	if ( S_OK != VariantCopy(dst,this))
		throw std::wstring(L"FVariant::ToCopy err");
}
void FVariant::FromCopy(const VARIANT* src) 
{
	clear_init();

	_ASSERT(vt == 0);
	InnerCopy(src);
}
void FVariant::set(const FVariant& var)
{
	clear_init();

	vt = var.vt;
	switch (var.vt) 
	{
		case VT_I8: setN(var.llVal);	break;
		case VT_BSTR: setS(var.bstrVal);break;
		case VT_UNKNOWN: setUnknown(var);	break;
		case VT_BOOL: setBL(var.boolVal); break;
		case VT_INT: setN(var.intVal);	break;
		case VT_I4: setN(var.intVal);	break;
		case VT_R8: setD(var.dblVal);	break;
		case VT_I2: setN(var.iVal);		break;
		case VT_R4: setD(var.fltVal);	break;
		case (VT_UNKNOWN|VT_BYREF):setUnknown(var);	break;
		default:
			THROW(L"not supported VT");
	}

	
}
double	FVariant::getD() const 
{ 
	if (vt == VT_R8)
		return dblVal; 
	else if (vt == VT_I8)
		return (double)llVal;
	else if (vt == VT_I4)
		return (double)intVal;
	else if (vt == VT_R4)
		return (double)fltVal;
	else if ( vt == VT_I2)
		return (double)iVal;
	else
		return 0.0;
}
__int64	FVariant::getN() const
{
	if (vt == VT_R8)
		return (__int64)dblVal;
	else if (vt == VT_I8)
		return (__int64)llVal;
	else if (vt == VT_I4)
		return (__int64)intVal;
	else if (vt == VT_R4)
		return (__int64)fltVal;
	else if (vt == VT_I2)
		return (__int64)iVal;
	else
		return 0;
}

bool FVariant::getBL() const 
{ 
	if (vt == VT_BOOL)
		return (boolVal == -1); 	
	else if ( vt != VT_BSTR && vt != VT_UNKNOWN )
	{
		return (0 != getN());
	}
	else
		THROW(L"FVariant::getBL() err");		
}
ULONG FVariant::length() const
{ 
	if ( vt == VT_BSTR )
		return ::SysStringLen(bstrVal); 
	else if (vt == VT_UNKNOWN)
	{
		IVARIANTArrayImp* pv = dynamic_cast<IVARIANTArrayImp*>(punkVal);
		return pv->Count();
	}
	return 0;

}
const FVariant& FVariant::operator = (const FVariant& v) 
{ 
	if (this != &v && !lock_)
	{
		clear_init();
		InnerCopy(&v);

		vt = v.vt;
		lock_ = v.lock_;

	}
	return *this; 
}
const FVariant& FVariant::operator = (const VARIANT& v)
{
	if (this != &v && !lock_)
	{
		clear_init();

		InnerCopy(&v);

		vt = v.vt;
		lock_ = false;
	}
	return *this;
}

void FVariant::toStr()
{
	WCHAR cb[256];

	if ( vt == VT_BSTR)
		return;
	else if (vt == VT_I8)
	{				
		StringCbPrintf(cb,_countof(cb), L"%I64d", llVal);		
		setS(cb);
	}
	else if (vt == VT_R8)
	{		
		StringCbPrintf(cb, _countof(cb), L"%f", dblVal);
		setS(cb);
	}
	else if (vt == VT_I4 || vt == VT_INT)
	{
		StringCbPrintf(cb, _countof(cb), L"%d",intVal);
		setS(cb);
	}
	else if (vt == VT_BOOL)
	{
		StringCbPrintf(cb, _countof(cb), L"%s", (boolVal? L"true":L"false"));
		setS(cb);
	}
	else if (vt == VT_UNKNOWN)
	{
		std::wstringstream sm;

		IVARIANTAbstract* par = (IVARIANTAbstract*)punkVal;
		if (par->TypeId() == FVariantType::ARRAY)
		{
			IVARIANTArray* par1 = (IVARIANTArray*)par;
			StringCbPrintf(cb, _countof(cb), L"array[size=%d]---\r\n", par1->Count());
			sm << cb;
			for(UINT i=0; i < min(5,par1->Count()); i++)
			{
				VARIANT v;
				::VariantInit(&v);
				par1->Get(i, &v);
				FVariant fvar(v);
				fvar.toStr();
				if (fvar.vt == VT_BSTR)
					sm << L"[" << i << L"]:" << fvar.bstrVal << L"\r\n";
				else
					sm << L"[" << i << L"]:" << fvar.getN() << L"\r\n";
				::VariantClear(&v);
			}
			setS(sm.str());
		}
		else if (par->TypeId() == FVariantType::FUNCTIONPOINTER)
		{
			StringCbPrintf(cb, _countof(cb), L"function");
			setS(cb);
		}		
		else if (par->TypeId() == FVariantType::MAP)
		{			
			IVARIANTMap* par1 = (IVARIANTMap*)par;
			StringCbPrintf(cb, _countof(cb), L"map----\r\n");
			IVARIANTMap* pmap = (IVARIANTMap*)punkVal;

			VARIANT keys1;
			::VariantInit(&keys1);
			pmap->Keys(&keys1);

			sm << cb;
			auto par2 = (IVARIANTArray*)keys1.punkVal;
			for (UINT i = 0; i < par2->Count(); i++)
			{
				VARIANT v;
				::VariantInit(&v);
				par2->Get(i, &v);
				if (v.vt == VT_BSTR)
				{
					StringCbPrintf(cb, _countof(cb), L"key:%s,\r\n",  v.bstrVal);
					sm << cb;
				}
			}



			setS(sm.str());
		}
		else
			setS(L"UNKNOWN error");
	}	
	else
	{
		StringCbPrintf(cb, _countof(cb), L"toStr error, VT=%d", vt);
		setS(cb);

	}
}
FVariant FVariant::getAr(int idx) const
{
	_ASSERT(vt == VT_UNKNOWN);

	IVARIANTArray* par = (IVARIANTArray*)punkVal;

	VARIANT v;
	::VariantInit(&v);
	par->Get(idx,&v);

	return FVariant(v);
}
bool FVariant::getDic(const FVariant& key, FVariant* ret) const
{
	_ASSERT(vt == VT_UNKNOWN);
	IVARIANTMapImp* par = (IVARIANTMapImp*)punkVal;
	if (FVariantType::MAP==par->TypeId())
	{
		VARIANT v;
		::VariantInit(&v);
		if ( par->GetItem(key.bstrVal, &v))
		{
			*ret = v;
			return true;
		}
	}
	return false;
}

void FVariant::setAr(const std::vector<FVariant>& ar)
{
	clear_init();
	
	IVARIANTArrayImp* par = new IVARIANTArrayImp();

	for(auto v : ar)
	{
		par->Add(v);
	}

	vt = VT_UNKNOWN;
	punkVal = par;
}


void FVariant::setMap(std::map<wstring, FVariant>& map)
{
	clear_init();

	IVARIANTMapImp* pmap = new IVARIANTMapImp();

	for(auto& it : map)
	{
		//VARIANT v = it.second.ToVARIANT();
		//pmap->SetItem(it.first.c_str(), v);



		pmap->SetItem2(it.first.c_str(), it.second);

		
	}

	vt = VT_UNKNOWN;
	punkVal = pmap;
}


void FVariant::setUnknown(const FVariant& var)
{
	clear_init();
	_ASSERT((var.vt&VT_UNKNOWN) == VT_UNKNOWN);
	InnerCopy(&var);
}

void FVariant::setDic(const FVariant& key, const FVariant& val)
{
	//clear();
	//init();

	_ASSERT(key.vt == VT_BSTR);

	if (vt == VT_EMPTY)
	{
		IVARIANTMapImp* par = new IVARIANTMapImp();
		
		BSTR ckey = key.bstrVal;
		VARIANT x = val.ToVARIANT();
		par->SetItem( ckey, x);

		vt = VT_UNKNOWN;
		punkVal = par;
	}
	else if (vt == VT_UNKNOWN)
	{
		IVARIANTMapImp* par = (IVARIANTMapImp*)punkVal;
		if (par->TypeId() == FVariantType::MAP)
		{
			BSTR ckey = key.bstrVal;
			VARIANT x = val.ToVARIANT();
			par->SetItem(ckey, x);
		}
	}

}
void FVariant::setAr(const FVariant& key, const FVariant& var)
{
	_ASSERT(vt == VT_UNKNOWN);

	IVARIANTArray* par = (IVARIANTArray*)punkVal;
	int id = par->TypeId();
	if ( id == FVariantType::ARRAY)
	{
		VARIANT fv = var.ToVARIANT();
		par->Set((int)key.llVal, fv );
	}
	else if ( id == FVariantType::MAP)
	{
		IVARIANTMap* par = (IVARIANTMap*)punkVal;
		wstring ckey = (LPCWSTR)key.bstrVal;
		
		VARIANT v = var.ToVARIANT();
		
		par->SetItem(ckey.c_str(), v);
	}

}

////
FVariant operator + (const FVariant& var1, const FVariant& var2)
{
	FVariant var;

	if (var1.vt == VT_R8 || var1.vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		var.setD(var1.getD() + var2.getD());
	else if (var1.vt == VT_I8 || var1.vt == VT_I4 || var1.vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		var.setN(var1.getN() + var2.getN());
	else if (var1.vt == VT_BSTR && var1.vt == var2.vt)
	{
		std::wstring s = var1.getSS();
		s += var2.getS();
		var.setS(s);
	}
	else
		THROW(L"FVariant::operator plus err");

	return var;
}
FVariant operator - (const FVariant& var1, const FVariant& var2)
{
	FVariant var;
	if (var1.vt == VT_R8 || var1.vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		var.setD(var1.getD() - var2.getD());
	else if (var1.vt == VT_I8 || var1.vt == VT_I4 || var1.vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		var.setN(var1.getN() - var2.getN());
	else
		THROW(L"FVariant::operator minus err");

	return var;
}

FVariant operator * (const FVariant& var1, const FVariant& var2)
{
	FVariant var;

	if (var1.vt == VT_R8 || var1.vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		var.setD(var1.getD() * var2.getD());
	else if (var1.vt == VT_I8 || var1.vt == VT_I4 || var1.vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		var.setN(var1.getN() * var2.getN());
	else
		THROW(L"FVariant::operator * err");



	return var;
}
FVariant operator / (const FVariant& var1, const FVariant& var2)
{
	FVariant var;

	if (var1.vt == VT_R8 || var1.vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
	{
		if (var2.getD() == 0)
			THROW(L"FVariant::operator / zero err");
		var.setD(var1.getD() / var2.getD());
	}
	else if (var1.vt == VT_I8 || var1.vt == VT_I4 || var1.vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
	{
		if ( var2.getN() == 0 )
			THROW(L"FVariant::operator / zero err");
		var.setN(var1.getN() / var2.getN());
	}
	else
		THROW(L"FVariant::operator / err");

	return var;
}
FVariant operator % (const FVariant& var1, const FVariant& var2)
{
	FVariant var;
	if (var1.vt == VT_I8 || var1.vt == VT_I4 || var1.vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		var.setN(var1.getN() % var2.getN());
	else
		THROW(L"FVariant::operator % err");

	return var;
}
BOOL FVariant::operator > (const FVariant& var2) const
{
	if (vt == VT_R8 || vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		return (getD() > var2.getD());
	else if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		return (getN() > var2.getN());
	else
		THROW(L"FVariant::operator > err");

	return FALSE;
}
BOOL FVariant::operator < (const FVariant& var2) const
{
	if (vt == VT_R8 || vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		return (getD() < var2.getD());
	else if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		return (getN() < var2.getN());
	else
		THROW(L"FVariant::operator < err");

	return FALSE;
}

BOOL FVariant::operator != (const FVariant& var2) const
{
	return !(*this == var2);
}
BOOL FVariant::operator == (const FVariant& var2) const
{
	if (vt == VT_BOOL && VT_BOOL == var2.vt)
		return (getBL() == var2.getBL());
	else if (vt == VT_BSTR && vt == var2.vt)
	{
		return ( wcscmp(bstrVal, var2.bstrVal) == 0); 
	}
	else if (vt == VT_R8 || vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		return (getD() == var2.getD());
	else if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		return (getN() == var2.getN());
	else if (vt == VT_UNKNOWN && var2.vt == VT_UNKNOWN)
		return (punkVal == var2.punkVal);
	else
		THROW(L"FVariant::operator == err" );

	return FALSE;
}

BOOL FVariant::operator <= (const FVariant& var2) const
{
	if (vt == VT_R8 || vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		return (getD() <= var2.getD());
	else if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		return (getN() <= var2.getN());
	else
		THROW(L"FVariant::operator <= err");

	return FALSE;
}

BOOL FVariant::operator >= (const FVariant& var2) const
{
	if (vt == VT_R8 || vt == VT_R4 || var2.vt == VT_R8 || var2.vt == VT_R4)
		return (getD() >= var2.getD());
	else if (vt == VT_I8 || vt == VT_I4 || vt == VT_I2 || var2.vt == VT_I8 || var2.vt == VT_I4 || var2.vt == VT_I2)
		return (getN() >= var2.getN());
	else
		THROW(L"FVariant::operator >= err");

	return FALSE;
}

FVariant operator || (const FVariant& var1, const FVariant& var2)
{
	return FVariant(var1.getBL() || var2.getBL());
}

FVariant operator&&(const FVariant& var1, const FVariant& var2)
{
	return FVariant(var1.getBL() && var2.getBL());
}


FVariant& FVariant::operator += (const FVariant& v)
{
	if (this == &v) return *this;
	FVariant vv(*this);
	vv = vv + v;
	set(vv);
	return *this;

}
FVariant& FVariant::operator += (LPCTSTR str)
{
	FVariant vv(*this);
	FVariant v;
	v.setS(str);
	vv = vv + v;
	set(vv);
	return *this;
}
