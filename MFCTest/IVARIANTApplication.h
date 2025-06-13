#pragma once

#include "lib/CVariantTool.h"

class CMFCTestView;

ULONG UnixTime(int yyyy, int mm, int dd);

class IVARIANTApplication : public IVARIANTAbstract
{
public:
	IVARIANTApplication(CMFCTestView* pview) :pview_(pview) {};
public:
	HWND parent_;
public:
	virtual void Clear() {}
	virtual int TypeId()  const { return 1000; }

	CMFCTestView* pview_;

	VARIANT create_object(VARIANT vid, VARIANT typ, VARIANT v);

public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
		if (riid == IID_IUnknown) {
			*ppv = static_cast<IUnknown*>(this);
		}
		else {
			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	virtual VARIANT Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt) override
	{
		std::wstring funcnm = cfuncnm;
		if (funcnm == L"create_object" && vcnt > 2)
		{
			return create_object(v[0], v[1], v[2]);
		}
		else if (funcnm == L"unixtime" && vcnt > 2)
		{
			VARIANT ret = {};
			ret.llVal = UnixTime(v[0].intVal, v[1].intVal, v[2].intVal);
			ret.vt = VT_I8;

			return ret;
		}
		else if (funcnm == L"tool")
		{
			
			return Tool();

		}
		throw(std::wstring(L"Invoke err"));

	}

	VARIANT Tool()
	{
		VARIANT v;
		::VariantInit(&v);

		v.punkVal = new CVARIANTTool();
		v.vt = VT_UNKNOWN;

		return v;
	}
};