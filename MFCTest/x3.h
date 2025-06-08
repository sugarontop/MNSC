#pragma once
#include "x.h"
#include "mnsc.h"
#include <sstream>
#include <msctf.h>
#include "tsf\TextEditor.h"
#include "tsf\IBridgeTSFInterface.h"

class IVARIANTTextbox : public IVARIANTAbstract, public IBridgeTSFInterface, public DrawingObject
{
public:
	IVARIANTTextbox(const CRect& rc, bool singleline):rc_(rc)
	{
		ct_.bSingleLine_ = singleline; //true;
		bActive_ = false;
		auto k = ctrl();
		k->SetContainer(&ct_, this);
	}

	virtual void Draw(CDC* pDC)
	{
		CFont cf;
		cf.CreatePointFont(210, L"Meiryo UI");
		auto oldf = pDC->SelectObject(&cf);

		if (bActive_)
		{
			ctrl()->Draw(*pDC);
		}
		else if ((HBITMAP)bmpText_ == nullptr && (int)ct_.GetTextLength() > -1 )
		{
			pDC->FillSolidRect(rc_, RGB(255,255,255));
			pDC->DrawTextW(ct_.GetTextBuffer(), ct_.GetTextLength(), &rc_, (int)DT_TOP | DT_LEFT);
		}
		else
		{
			CDC cDC;
			cDC.CreateCompatibleDC(pDC);
			auto old = cDC.SelectObject(&bmpText_);

			pDC->BitBlt(rc_.left,rc_.top,rc_.Width(),rc_.Height(), &cDC, 0,0, SRCCOPY);

			cDC.SelectObject(old);
			cDC.DeleteDC();
		}


		//if (border_)
		{
			CBrush br;
			br.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));
			CRect rc1(rc_);
			rc1.InflateRect(1,1);
			pDC->FrameRect(rc1, &br);
		}

		pDC->SelectObject(oldf);

	}
	virtual void setText(const std::wstring& txt)
	{
		
	}
	VARIANT setText(VARIANT  txt)
	{

		if (txt.vt == VT_BSTR)
		{
			LPCWSTR str = txt.bstrVal;
			UINT cnt = lstrlen(str);
			UINT result = 0;
			ct_.Reset();
			ct_.InsertText(0, str, cnt, result, false );

			bmpText_.DeleteObject();

		}

		_variant_t ret(0);
		return ret;
	}
	void SetFocus(bool bFocus)
	{
		if (bFocus)
		{
			mat_._31 = (float)rc_.left;
			mat_._32 = (float)rc_.top;

			bActive_ = true;
			ctrl()->SetContainer(&ct_, this);
			ctrl()->SetFocus(&mat_);
		}
		else
		{
			bActive_ = false;

			ctrl()->CopyBitmap(&bmpText_);
			ctrl()->layout_.Clear();
		}

	}
	void setProperty(VARIANT brd, VARIANT readonly)
	{
		

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
