#pragma once
#include "x.h"
#include "mnsc.h"
#include <sstream>
#include <msctf.h>
#include "tsf\TextEditor.h"
#include "tsf\IBridgeTSFInterface.h"

BOOL PushAxisAlignedClip(CDC* pDC, const CRect& clipRect);
BOOL PopAxisAlignedClip(CDC* pDC);


class IVARIANTTextbox : public IVARIANTAbstract, public IBridgeTSFInterface, public DrawingObject
{
public:
	IVARIANTTextbox(const CRect& rc, BOOL multiline):rc_(rc)
	{
		ct_.bSingleLine_ = !multiline;
		bActive_ = false;
		auto k = ctrl();
		k->SetContainer(&ct_, this);

		fontheight_ = 21;
		border_ = true;
		readonly_ = false;
		fontname_ = L"Meiryo UI";
	}

	virtual void Draw(CDC* pDC)
	{		
		CRect rc1=rc_;
		rc1.InflateRect(1,1);

		// 領域フィルタ
		PushAxisAlignedClip(pDC, rc1);

		// フォント設定
		CFont cf;
		cf.CreatePointFont(fontheight_*10, fontname_.c_str());
		auto oldf = pDC->SelectObject(&cf);
		if (bActive_)
		{
			// focus時、文字表示、スクロールバー表示、キャレット表示
			ctrl()->Draw(*pDC, readonly_);
		}
		else
		{
			// unfocus時、確定した文字のbitmap表示
			CDC cDC;
			cDC.CreateCompatibleDC(pDC);
			auto old = cDC.SelectObject(&bmpText_);

			pDC->BitBlt(rc_.left,rc_.top,rc_.Width(),rc_.Height(), &cDC, 0,0, SRCCOPY);

			cDC.SelectObject(old);
			cDC.DeleteDC();
		}

		// 枠線の表示
		if (border_)
		{
			CBrush br;
			br.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));

			pDC->OffsetViewportOrg(rc_.left, rc_.top);

			CRect rc2(0,0,rc_.Width(), rc_.Height());
			rc2.InflateRect(1,1);
			pDC->FrameRect(rc2, &br);

			pDC->OffsetViewportOrg(-rc_.left, -rc_.top);
		}

		pDC->SelectObject(oldf);
		PopAxisAlignedClip(pDC);
	}
	virtual void setText(const std::wstring& str)
	{				
		setTextInner(str.c_str(), str.length());
	}

	void setTextInner(LPCWSTR str, UINT cnt)
	{
		UINT result = 0;
		ct_.Reset();
		ct_.InsertText(0, str, cnt, result, false);

		bmpText_.DeleteObject();
	}

	VARIANT setText(VARIANT  txt)
	{
		if (txt.vt == VT_BSTR)
		{
			LPCWSTR str = txt.bstrVal;
			setTextInner(str, lstrlen(str));
		}
		else if (txt.vt == VT_INT || txt.vt == VT_I8)
		{
			WCHAR cb[64];
			wsprintf(cb,L"%d", txt.intVal);
			setTextInner(cb, lstrlen(cb));
		}
		else if (txt.vt == VT_UNKNOWN)
		{
			std::wstring cb = L"VT_UNKNOWN type";
			setTextInner(cb.c_str(),cb.length());
		}

		return _variant_t(0).Detach();
	}
	void SetFocus(bool bFocus)
	{
		if (bFocus)
		{
			bActive_ = true;

			mat_._31 = (float)rc_.left;
			mat_._32 = (float)rc_.top;

			ctrl()->SetContainer(&ct_, this);
			ctrl()->SetFocus(&mat_);
		}
		else
		{
			bActive_ = false;

			ctrl()->CopyBitmap(&bmpText_);
			ctrl()->layout_.Clear();
			
			
			
			DestroyCaret();
		}

	}
	void setProperty(IVARIANTMap* map)
	{
		_variant_t brd, readonly, fontheight,text, fontnm;

		if (map->GetItem(L"readonly", &readonly) && readonly.vt == VT_BOOL)
			readonly_ = readonly.boolVal;
		
		if (map->GetItem(L"border", &brd) && brd.vt == VT_BOOL)
			border_ = brd.boolVal;
		
		if (map->GetItem(L"fontheight", &fontheight) && fontheight.vt == VT_INT)
			fontheight_ = fontheight.intVal;

		if (map->GetItem(L"fontname", &fontnm) && fontnm.vt == VT_BSTR)
			fontname_ = fontnm.bstrVal;

		if (map->GetItem(L"text", &text) && text.vt == VT_BSTR)
			setText(text);
	}
	bool ReadOnly() const { return readonly_; }

	bool ScrollByWheel(bool bup)
	{
 		return ctrl()->ScrollByWheel(bup);
	}

	virtual void setRect(const CRect& rc) { rc_ = rc; }
	virtual CRect getRect() { return rc_; }

	virtual void Clear() {}
	virtual int TypeId()  const { return 2002; }
	TSF::CTextEditorCtrl* ctrl() const;
	

	CRect rc_;
	TSF::CTextContainer ct_;
	V6::D2DMat mat_;
	bool bActive_;
	CBitmap bmpText_;
	int fontheight_;
	std::wstring fontname_;
	bool border_;
	bool readonly_;
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
			VARIANT ret = setText(v[0]);

			SetFocus(true);
			return ret;
		}
		else if (funcnm == L"setprop" && vcnt > 0)
		{
			if ( v[0].vt == VT_UNKNOWN )
			{
				auto p = (IVARIANTAbstract*)v[0].punkVal;

				if (p->TypeId() == 2 ) //FVariantType::MAP)
				{
					IVARIANTMap* pm = (IVARIANTMap*)p;

					setProperty(pm);

					SetFocus(true);

					Redraw();

					return _variant_t(0).Detach();
				}
			}
		}
		
		std::wstring err = L"Invoke err:";
		err += cfuncnm;

		throw(err);
	}
	void Redraw()
	{
		ctrl()->InvalidateRect();
	}

public :
	virtual CRect GetClientRect() const{ return rc_; }
	virtual IDWriteTextFormat* GetFormat() const { return nullptr; }
	virtual TYP GetType() const { return (ct_.bSingleLine_ ? IBridgeTSFInterface::SINGLELINE : IBridgeTSFInterface::MULTILINE); };
};


struct TSFIsland
{
	HWND hWnd;
	TfClientId TfClientId;
	CComPtr<ITfThreadMgr2> pThreadMgr;
	CComPtr<ITfKeystrokeMgr> pKeystrokeMgr;
	CComPtr<ITfDisplayAttributeMgr> DisplayAttributeMgr;
	void* ctrl;
};

bool TSFInit(HWND hWnd);
void TSFClose(HWND hWnd);
