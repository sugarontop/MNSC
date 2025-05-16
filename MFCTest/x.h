#pragma once

class DrawingObject
{
public:
	DrawingObject(CRect rc) :rc_(rc)
	{

	}
	virtual void Draw(CDC* pDC)
	{
		pDC->Rectangle(rc_);

		pDC->DrawTextExW(
			const_cast<LPWSTR>(text_.c_str()), // std::wstring の c_str() を LPWSTR にキャスト  
			static_cast<int>(text_.length()), // size_t を int にキャスト  
			&rc_,
			DT_VCENTER | DT_SINGLELINE | DT_CENTER,
			nullptr
		);



	}

	CRect rc_;
	std::wstring text_;
};


class IVARIANTButton : public IVARIANTAbstract
{
public:
	IVARIANTButton(DrawingObject* pview) :pview_(pview) {};
public:
	
public:
	virtual void Clear() {}
	virtual int TypeId() { return 2000; }

	DrawingObject* pview_;

	VARIANT setText(VARIANT  txt)
	{
		
		if (txt.vt == VT_BSTR)
		{
			pview_->text_ = txt.bstrVal;
		}


		CComVariant ret(0);
		return ret;
	}

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