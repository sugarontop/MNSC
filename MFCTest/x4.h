#pragma once
#include "x.h"
#include "lsbox.h"
class IVARIANTDropdownList : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTDropdownList(const CRect& rc) :rc_(rc), bShowListbox_(false)
	{
		CSize sz = rc.Size();

		CSize listbox_size( sz.cx, 30*4);
		CSize itemsz( sz.cx, 30);
		ls_.SetViewSize(listbox_size, itemsz);

		text_ = L"=====";
	}

private:
	CRect rc_;
	std::wstring text_;
	ListboxBase ls_;
	std::vector<std::wstring> items_;
	bool bShowListbox_;
public:
	_variant_t func_onselect_;

	virtual void Clear() {}
	virtual int TypeId()  const { return 2003; }

	virtual void Draw(CDC* pDC)
	{
		pDC->SaveDC();
		pDC->DrawTextExW(
				const_cast<LPWSTR>(text_.c_str()),
				static_cast<int>(text_.length()),
				&rc_,
				DT_VCENTER | DT_SINGLELINE,
				nullptr
			);
		
		CRect btn( rc_.right-20,rc_.top, rc_.right, rc_.bottom);

		pDC->FillSolidRect(btn, RGB(170,170,170));

		
		if (bShowListbox_)
		{					
			pDC->OffsetViewportOrg(rc_.left, rc_.bottom);
			ls_.Draw(pDC);			
		}
		pDC->RestoreDC(-1);
	}
	virtual void setText(const std::wstring& txt) { text_ = txt; }
	virtual VARIANT setText(VARIANT  txt)
	{ 
		if (txt.vt == VT_UNKNOWN)
		{
			
			IVARIANTArray* par = (IVARIANTArray*)txt.punkVal;


			for (UINT i = 0; i < par->Count(); i++)
			{
				VARIANT v;
				::VariantInit(&v);
				if (par->Get(i, &v))
				{
					if (v.vt == VT_BSTR)
					{
						items_.push_back(v.bstrVal);
					}
				}
				::VariantClear(&v);
			}

			ls_.SetString(items_);
		}
	

		CComVariant ret(0);
		return ret;
	}

	


	bool BtnClick(CPoint pt)
	{
		CRect btn=rc_; //(rc_.right-20, rc_.top, rc_.right, rc_.bottom);
		return btn.PtInRect(pt);
	}

	void ShowDlgListbox(bool bShow)
	{
		bShowListbox_ = bShow;

		if ( !bShow )
			text_ = ls_.GetSelectString();
	}

	int SelectRow(CPoint point, bool bLast=false)
	{
		point.x -= rc_.left;
		point.y -= rc_.bottom;


		if (point.y < 0)
			return 2;

		int a = ls_.GetSelectIdx();
			
		int md = ls_.SelectRowTest(point, bLast);
		
		if (md == 0 && bLast)
			ShowDlgListbox(false);

		if (md == 1 && bLast)
		{
			int b = ls_.GetSelectIdx();

			if (a != b)
			{
				if (func_onselect_.vt == VT_UNKNOWN)
				{
					IVARIANTFunction* func = (IVARIANTFunction*)func_onselect_.punkVal;

					VARIANT v1;
					::VariantInit(&v1);
					v1.vt = VT_UNKNOWN;
					v1.punkVal = this;

					VARIANT v2 = func->Invoke(L"NONAME", &v1, 1);
					::VariantClear(&v2);
				}
			}
		}
		
		return md;
	}
	bool ScrollByWheel(bool bdown)
	{
		return ls_.ScrollByWheel(bdown);
	}
	void ScrollbarYoff(int off)
	{
		ls_.ScrollbarYoff(off);
	}
	CRect RectAll() const
	{
		CRect rc = rc_;
		rc.bottom = rc.bottom + ls_.Size().cy;
		return rc;
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
		else if (funcnm == L"selectidx")
		{
			VARIANT v;
			::VariantInit(&v);
			v.vt = VT_INT;
			v.intVal = (int)ls_.GetSelectIdx();
			return v;
		}
		else if (funcnm == L"gettext" && vcnt > 0)
		{
			VARIANT v1 = v[0];
			VARIANT ret;
			::VariantInit(&ret);

			int idx = v1.intVal;

			if (0 <= idx && idx < items_.size())
			{
				auto s = items_[idx];

				ret.vt = VT_BSTR;
				ret.bstrVal = ::SysAllocString(s.c_str());
			}

			return ret;
		}

		throw(std::wstring(L"Invoke err"));
	}
}; 
