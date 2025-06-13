#pragma once
#include "x.h"

void DrawStockChart(CDC& cDC, const std::vector<VARIANT>& days, const std::vector<VARIANT>& open, const std::vector<VARIANT>& high, const std::vector<VARIANT>& low, const std::vector<VARIANT>& close, CSize viewSize);



std::vector<VARIANT> Convert(IVARIANTArray& sar);
std::vector<VARIANT> ConvertUnixDate(IVARIANTArray& sar);

BOOL PushAxisAlignedClip(CDC* pDC, const CRect& clipRect);
BOOL PopAxisAlignedClip(CDC* pDC);


class IVARIANTCanvas : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTCanvas(const CRect& rc) :rc_(rc),border_(1), data_stat_(0)
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
		pDC->SaveDC();

		if (data_stat_ == 0)
		{
			pDC->DrawTextExW(
					const_cast<LPWSTR>(text_.c_str()),
					static_cast<int>(text_.length()),
					&rc_,
					DT_LEFT | DT_SINGLELINE|DT_TOP,
					nullptr
				);
		}
		else
		{
		
			auto& date = *(IVARIANTArray*)(IUnknown*)data_[0];
			auto& ls2 = *(IVARIANTArray*)(IUnknown*)data_[1];
			auto& ls3 = *(IVARIANTArray*)(IUnknown*)data_[2];
			auto& ls4 = *(IVARIANTArray*)(IUnknown*)data_[3];
			auto& ls5 = *(IVARIANTArray*)(IUnknown*)data_[4];
		
			
			PushAxisAlignedClip(pDC, rc_);

			pDC->OffsetViewportOrg(rc_.left, rc_.top);
			DrawStockChart(*pDC, ConvertUnixDate(date), Convert(ls2), Convert (ls3), Convert (ls4), Convert(ls5),CSize(rc_.Width(),rc_.Height()));


			pDC->OffsetViewportOrg(-rc_.left, -rc_.top);

			PopAxisAlignedClip(pDC);


			date.Release();
			ls2.Release();
			ls3.Release();
			ls4.Release();
			ls5.Release();
		}

		if (border_)
		{
			CBrush br;
			br.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));
			pDC->FrameRect(rc_, &br);
		}

		pDC->RestoreDC(-1);
	}
	virtual void setText(const std::wstring& txt) 
	{ 
		text_ = txt; 		
	}

	virtual void setRect(const CRect& rc) { rc_ = rc; }
	virtual CRect getRect() { return rc_; }

	
	int data_stat_;
	_variant_t data_[5];

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
		else if ( funcnm == L"setdata" && vcnt == 5 )
		{
			for(int i=0; i < 5;i++)
			{
				data_[i] = v[i];
				::VariantClear(&v[i]);
			}

			data_stat_ = 1;

			return _variant_t(0).Detach();
		}


		throw(std::wstring(L"Invoke err"));

	}
}; 
