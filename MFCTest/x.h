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
	IVARIANTButton(CRect& rc) :rc_(rc), enable_(VARIANT_TRUE)
	{
		
	}

	~IVARIANTButton(){}
public:
	_variant_t func_onclick_;
public:
	virtual void Clear() { func_onclick_.Clear();}
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
		auto old = pDC->SetBkMode(TRANSPARENT);
		auto color = (enable_ == VARIANT_TRUE ? RGB(255, 255, 255) : RGB(210, 210, 210));

		CPen pen(PS_SOLID, 1, RGB(0, 0, 0)); // •‚¢ŽÀüA•1
		CPen* pOldPen = pDC->SelectObject(&pen);

		CBrush brush(color); 
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		pDC->Rectangle(rc_);
		
		pDC->DrawTextExW(
			const_cast<LPWSTR>(text_.c_str()),
			static_cast<int>(text_.length()),
			&rc_,
			DT_VCENTER | DT_SINGLELINE | DT_CENTER,
			nullptr
		);
		

		pDC->SetBkMode(old);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);

		
	}

	virtual void setText(const std::wstring& txt) { text_ = txt; }

	virtual void setRect(const CRect& rc){ rc_=rc;}
	virtual CRect getRect(){ return rc_; }


	CRect rc_;
	std::wstring text_;
	VARIANT_BOOL enable_;
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
		else if (funcnm == L"enable" && vcnt > 0)
		{
			enable_ = VARIANT_FALSE;
			if ( v[0].vt == VT_BOOL )
				enable_ = v[0].boolVal;
			else if (v[0].vt == VT_INT || v[0].vt == VT_I8)
				enable_ = (v[0].lVal==0 ? VARIANT_FALSE : VARIANT_TRUE);

			VARIANT xv;
			xv.vt = VT_INT;
			xv.intVal = 0;
			return xv;
		}

		throw(std::wstring(L"Invoke err"));

	}
};