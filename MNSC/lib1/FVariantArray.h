#pragma once
#include "mnsc.h"
#include "FVariant.h"


class CPrsFunction;

class IVARIANTArrayImp : public IVARIANTArray
{
public:
	IVARIANTArrayImp() {};

public:
	std::vector<VARIANT> ar;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	virtual int TypeId() { return FVariantType::ARRAY; }
	virtual void Add(VARIANT& v);
	virtual void Clear();
	virtual bool Get(int idx, VARIANT*);
	virtual ULONG Count() { return (ULONG)ar.size(); }
	virtual void Set(int idx, VARIANT& v);
	virtual VARIANT Invoke(LPCWSTR funcnm, VARIANT* v, int vcnt);
};

class IVARIANTMapImp : public IVARIANTMap
{
public:
	IVARIANTMapImp();
	~IVARIANTMapImp() { Clear(); }

public:	
	std::unordered_map<std::wstring, FVariant> map;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	virtual int TypeId() { return FVariantType::MAP; }
	
	virtual ULONG Count() { return (ULONG)map.size(); }	
	virtual VARIANT Invoke(LPCWSTR funcnm, VARIANT* v, int vcnt);
	virtual void SetItem(LPCWSTR key, VARIANT& v)
	{	
		// v‚ð‹zŽû‚·‚é‚Ì‚ÅAVariantClear‚Í•s—v
		map[key] =v;

	}
	virtual bool GetItem(LPCWSTR key, VARIANT* v)
	{
		if ( map.find(key) != map.end())
		{
			FVariant xv = map[key];
			if ( xv.vt >= VT_RESERVED )
			{
				v->vt = xv.vt;
				return true;
			}
			else
			{
				*v = xv.ToVARIANT();
				return true;
			}
		}
		return false;
	}
	virtual UINT Keys(VARIANT* ret)
	{
		auto ar = new IVARIANTArrayImp();
		for(auto& it : map)
		{
			VARIANT v;
			::VariantInit(&v);
			v.bstrVal = ::SysAllocString(it.first.c_str());
			v.vt = VT_BSTR;
			ar->Add(v);
		}

		
		::VariantInit(ret);
		ret->punkVal = ar;
		ret->vt = VT_UNKNOWN;
		
		return (UINT)map.size();
	}
	virtual void Clear(){}
};

class IVARIANTFunctionImp : public IVARIANTAbstract
{
public:
	IVARIANTFunctionImp();
private:
	std::shared_ptr<CPrsFunction> func_;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	virtual int TypeId() { return FVariantType::FUNCTIONPOINTER; }
	virtual void Clear();
	virtual VARIANT Invoke(LPCWSTR funcnm, VARIANT* v, int vcnt);
public:
	virtual void SetItem(std::shared_ptr<CPrsFunction> func)
	{
		func_ = func;
	}
	virtual std::shared_ptr<CPrsFunction> GetItem()
	{
		return func_;
	}
};
