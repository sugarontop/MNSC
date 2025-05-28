#pragma once
#include "x.h"

class IVARIANTCanvas : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTCanvas(const CRect& rc) :rc_(rc),border_(1)
	{
	}

private:
	CRect rc_;
	std::wstring text_;
	int border_;
public:
	virtual void Clear() {}
	virtual int TypeId()  const { return 2007; }

	VARIANT setText(VARIANT  txt)
	{
		if (txt.vt == VT_BSTR)
		{
			text_ = txt.bstrVal;
		}
		_variant_t ret(0);
		return ret;
	}
	VARIANT setProperty(VARIANT border, VARIANT readonly)
	{
		if ( border.vt == VT_INT)
			border_ = border.intVal;
		else if (border.vt == VT_I8 )
			border_ = (int)border.llVal;

		
		_variant_t ret(0);
		return ret;
	}
	
public :

	 

	virtual void Draw(CDC* pDC)
	{
		//pDC->FillSolidRect(rc_,(readonly_ ? RGB(210,210,210): RGB(255, 255, 255)));

		pDC->DrawTextExW(
				const_cast<LPWSTR>(text_.c_str()),
				static_cast<int>(text_.length()),
				&rc_,
				DT_LEFT | DT_SINGLELINE|DT_TOP,
				nullptr
			);


		if (border_)
		{
			CBrush br;
			br.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));
			pDC->FrameRect(rc_, &br);
		}
	}
	virtual void setText(const std::wstring& txt) 
	{ 
		text_ = txt; 		
	}

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
