#pragma once

#include "mnsc.h"


#define WM_BRADCAST_SET_INIT (WM_APP+10)


class DrawingObject
{
	public:

		DrawingObject(){}
		virtual void Draw(CDC* pDC)=0;
		virtual void setRect(const CRect& rc) = 0;
		virtual CRect getRect() = 0;
		virtual void setText(const std::wstring& txt) = 0;
};


class IVARIANTButton : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTButton(CRect& rc) :rc_(rc)
	{
		int a = 0;
	}

	~IVARIANTButton(){}
public:
	
public:
	virtual void Clear() {}
	virtual int TypeId() const { return 2000; }

	

	VARIANT setText(VARIANT  txt)
	{
		
		if (txt.vt == VT_BSTR)
		{
			text_ = txt.bstrVal;
		}


		CComVariant ret(0);
		return ret;
	}
	virtual void Draw(CDC* pDC)
	{
		pDC->Rectangle(rc_);

		pDC->DrawTextExW(
			const_cast<LPWSTR>(text_.c_str()),
			static_cast<int>(text_.length()),
			&rc_,
			DT_VCENTER | DT_SINGLELINE | DT_CENTER,
			nullptr
		);
	}
	virtual void setText(const std::wstring& txt) { text_ = txt; }

	virtual void setRect(const CRect& rc){ rc_=rc;}
	virtual CRect getRect(){ return rc_; }


	CRect rc_;
	std::wstring text_;
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
		if (funcnm == L"settext" && vcnt > 0)
		{
			return setText(v[0]);
		}

		throw(std::wstring(L"Invoke err"));

	}
};