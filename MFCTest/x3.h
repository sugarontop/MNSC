#pragma once
#include "x.h"

class IVARIANTTextbox : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTTextbox(const CRect& rc) :rc_(rc)
	{
	}

private:
	CRect rc_;
	std::wstring text_;

public:
	virtual void Clear() {}
	virtual int TypeId() { return 2002; }

	std::vector<std::wstring> items;

	VARIANT setText(VARIANT  txt)
	{
		if (txt.vt == VT_BSTR)
		{
			text_ = txt.bstrVal;
		}


		CComVariant ret(0);
		return ret;
	}
	const int item_h = 21;

	virtual void Draw(CDC* pDC)
	{
		CFont cf;
		cf.CreatePointFont(item_h * 6, L"Meiryo UI");

		CFont* old = pDC->SelectObject(&cf);
		pDC->DrawTextExW(
				const_cast<LPWSTR>(text_.c_str()),
				static_cast<int>(text_.length()),
				&rc_,
				DT_VCENTER | DT_SINGLELINE,
				nullptr
			);
			
			
		

		
		//pDC->Rectangle(rc_);
		pDC->SelectObject(old);
	}

	virtual void setText(const std::wstring& txt) { text_ = txt; }

	virtual void setRect(const CRect& rc) { rc_ = rc; }
	virtual CRect getRect() { return rc_; }



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
