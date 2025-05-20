#pragma once
#include "x.h"
#include "mnsc.h"


class IVARIANTListbox : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTListbox(const CRect& rc) :rc_(rc), select_idx_(-1), top_idx_(0), vscroll_(false), vscroll_move_(false), offy_(0)
	{
	}
	~IVARIANTListbox()
	{
		Clear();
	}
	
private:
	int select_idx_, top_idx_, offy_;
	bool vscroll_, vscroll_move_;
	CRect rc_;
	std::wstring text_; 
	
public:
	_variant_t func_onselect_;


	virtual void Clear() { func_onselect_.Clear(); }
	virtual int TypeId() { return 2001; }

	std::vector<std::wstring> items;
	
	VARIANT setText(VARIANT  txt)
	{
		if (txt.vt == VT_UNKNOWN)
		{
			IVARIANTArray* par = (IVARIANTArray*)txt.punkVal;
			

			for (int i = 0; i < par->Count(); i++)
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

			item_h* items.size() > rc_.Height() ? vscroll_ = true : vscroll_ = false;
		}


		CComVariant ret(0);
		return ret;
	}
	const int item_h = 21;
	const int vscbar_w = 20;
	const int real_item_h = (item_h + 2);

	virtual void Draw(CDC* pDC)
	{
		CRect rc(0,0,rc_.Width(), rc_.Height());

		CBrush brush; // ブラシを作成
		brush.CreateStockObject(NULL_BRUSH); // 空のブラシを選択
		CBrush* oldbr = pDC->SelectObject(&brush);

		
		CFont cf;
		cf.CreatePointFont(item_h*6, L"Arial");

		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC, rc_.Width(), rc_.Height());
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);
		CBitmap* pOldBitmap = memDC.SelectObject(&bmp);
		memDC.FillSolidRect(rc, RGB(250, 250, 250));

		CFont* oldfont = memDC.SelectObject(&cf);

		CRect rctext(0,0,rc_.Width(), item_h);
		for (int i = top_idx_; i < items.size(); i++)
		{			
			auto& it = items[i];

			memDC.FillSolidRect(rctext, (i==select_idx_ ? RGB(230, 230, 230) : RGB(255, 255, 255)));

			memDC.DrawTextExW(
				const_cast<LPWSTR>(it.c_str()),
				static_cast<int>(it.length()), 
				&rctext,
				DT_VCENTER | DT_SINGLELINE ,
				nullptr
			);
			rctext.OffsetRect(0, rctext.Height()+2);


			if (rctext.top > rc_.Height())
				break;
		}

		if (vscroll_)
		{
			CRect rcv(rc_.Width() - vscbar_w, 0, rc_.Width(), rc_.Height());
			memDC.FillSolidRect(rcv, RGB(240, 240, 240));

			int h = ScrollBarHeight(rc_.Height(), items.size() * (real_item_h)- rc_.Height() );
			CRect rcsv(rc_.Width() - vscbar_w, max(0, 0+offy_), rc_.Width(), h + offy_);
			memDC.FillSolidRect(rcsv, RGB(200, 200, 200));
		}


		memDC.SelectObject(oldfont);

		pDC->BitBlt(rc_.left, rc_.top, rc_.Width(), rc_.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBitmap);

		pDC->Rectangle(rc_);
		pDC->SelectObject(brush);
	}

	int ScrollBarHeight(int vh, int nvh)
	{
		float h = (float)vh * (float)vh/(nvh+vh);

		return (int)h;

	}

	int getItem(CPoint global_pt, CRect& rc)
	{
		CPoint pt(global_pt.x - rc_.left, global_pt.y - rc_.top); 
		
		CRect rcscrollbar(rc_.Width() - vscbar_w, 0, rc_.Width(), rc_.Height());

		if (rcscrollbar.PtInRect(pt))
		{			
			return -2;			
		}



		CRect rctext(0, 0, rc_.Width()- vscbar_w, item_h);

		

		int i = top_idx_;
		for (auto& it : items)
		{
			if ( rctext.PtInRect(pt))
			{
				rc = rctext;
				return i;
			}
			rctext.OffsetRect(0, rctext.Height() + 2);
			i++;
		}

		return -1;
	}

	void select(int idx)
	{
		select_idx_ = -1;
		if (idx < items.size())
		{
			if ( idx != select_idx_)
			{
				select_idx_ = idx;

				if (func_onselect_.vt == VT_UNKNOWN)
				{
					IVARIANTFunction* func = (IVARIANTFunction*)func_onselect_.punkVal;

					VARIANT v1;
					::VariantInit(&v1);
					v1.vt = VT_UNKNOWN;
					v1.punkVal = this;
					
					
					VARIANT v2 = func->Invoke(L"NONAME",&v1,1);

					//::VariantClear(&v1);
					::VariantClear(&v2);

				}
			}
			
			
		}
	}
	void scrollbar(bool bl)
	{
		vscroll_move_ = true;
	}

	void scrollbarMove(int offy)
	{
		if (!vscroll_move_) return;

		offy_ += offy;

		offy_ = max(0, offy_);

		int h = ScrollBarHeight(rc_.Height(), items.size() * (real_item_h) - rc_.Height());

		int c = offy_;
		int nvh = items.size() * real_item_h - rc_.Height();
		int b = rc_.Height() - h;

		float vc = ((float)c*nvh)/b;

		if (vc > nvh)
		{
			vc = nvh;
			offy_ -= offy;
		}

		top_idx_ = (int)(vc / real_item_h );
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
			select_idx_ = v[0].intVal;

			return v[0];

		}
		else if (funcnm == L"selectidx" )
		{
			VARIANT v;
			::VariantInit(&v); 
			v.vt = VT_I8;
			v.lVal =select_idx_;
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