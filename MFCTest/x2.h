#pragma once
#include "x.h"
#include "mnsc.h"
#include "lsbox.h"

class IVARIANTListbox : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTListbox(const CRect& rc) :rc_(rc)
	{
		CSize sz = rc.Size();

		CSize listbox_size(sz.cx, rc.Height());
		CSize itemsz(sz.cx, 30);
		ls_.SetViewSize(listbox_size, itemsz);

	}

private:
	CRect rc_;
	std::wstring text_; 
	ListboxBase ls_;
public:
	_variant_t func_onselect_;


	virtual void Clear() { func_onselect_.Clear(); }
	virtual int TypeId()  const { return 2001; }

	std::vector<std::wstring> items;
	
	VARIANT setText(VARIANT  txt)
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
						items.push_back(v.bstrVal);
					}
				}
				::VariantClear(&v);
			}
			ls_.SetString(items);			
		}


		CComVariant ret(0);
		return ret;
	}

	virtual void Draw(CDC* pDC)
	{
		pDC->SaveDC();
		pDC->OffsetViewportOrg(rc_.left, rc_.top);
		ls_.Draw(pDC);
		pDC->RestoreDC(-1);

	}
	int SelectRow(CPoint point, bool bLast=false)
	{
		point.x -= rc_.left;
		point.y -= rc_.top;

		int a = ls_.GetSelectIdx();

		int md = ls_.SelectRowTest(point, bLast);

		if ( md == 1 && bLast)
		{
			int b = ls_.GetSelectIdx();

			if ( a != b )
			{
				if (func_onselect_.vt == VT_UNKNOWN)
				{
					IVARIANTFunction* func = (IVARIANTFunction*)func_onselect_.punkVal;

					VARIANT v1;
					::VariantInit(&v1);
					v1.vt = VT_UNKNOWN;
					v1.punkVal = this;
					
					VARIANT v2 = func->Invoke(L"NONAME",&v1,1);

					::VariantClear(&v2);

				}

			}
		}
		return md;
	}
	
	void ScrollbarYoff(int off)
	{
		ls_.ScrollbarYoff(off);
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
		else if ( funcnm == L"select" && vcnt > 0)
		{
			int new_idx = v[0].intVal;
			int old_idx = ls_.GetSelectIdx();
			ls_.SetSelect(new_idx);

			if ( new_idx != old_idx )
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

			return v[0];

		}
		else if (funcnm == L"selectidx" )
		{
			VARIANT v;
			::VariantInit(&v); 
			v.vt = VT_I8;
			v.lVal =ls_.GetSelectIdx();
			return v;
		}
		else if (funcnm == L"gettext" && vcnt > 0)
		{
			VARIANT v1 = v[0];
			VARIANT ret;
			::VariantInit(&ret);

			int idx = v1.intVal;

			if (0 <= idx && idx < items.size())
			{
				auto s = items[idx];
			
				ret.vt = VT_BSTR;
				ret.bstrVal = ::SysAllocString(s.c_str());
			}

			return ret;
		}

		throw(std::wstring(L"Invoke err"));

	}
};