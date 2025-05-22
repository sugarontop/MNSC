
// MFCTestView.cpp : CMFCTestView クラスの実装
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "MFCTest.h"
#endif

#include "MFCTestDoc.h"
#include "MFCTestView.h"
#include "x.h"
#include "x2.h"
#include "x3.h"

#pragma comment(lib,"MNSC.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCTestView

IMPLEMENT_DYNCREATE(CMFCTestView, CView)

BEGIN_MESSAGE_MAP(CMFCTestView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CMFCTestView コンストラクション/デストラクション

CMFCTestView::CMFCTestView() noexcept
{
	// TODO: 構築コードをここに追加します。

}

CMFCTestView::~CMFCTestView()
{
}

BOOL CMFCTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}




// CMFCTestView の診断

#ifdef _DEBUG
void CMFCTestView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCTestDoc* CMFCTestView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCTestDoc)));
	return (CMFCTestDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCTestView メッセージ ハンドラー

VARIANT ScriptCall(ScriptSt & st, LPCWSTR funcnm, VARIANT * prms, int pmcnt)
{
	VARIANT v = MNSCCall(st,funcnm,prms,pmcnt);

	if (!st.result)
	{
		AfxMessageBox(st.error_msg);
		MNSCClose(st);
	}
	return v;
}


class IVARIANTApplication : public IVARIANTAbstract
{
public:
	IVARIANTApplication(CMFCTestView* pview) :pview_(pview) {};
public:
	HWND parent_;
public:
	virtual void Clear() {}
	virtual int TypeId() { return 1000; }

	CMFCTestView* pview_;

	VARIANT create_object(VARIANT typ, VARIANT v);

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
		if (funcnm == L"create_object" && vcnt > 0)
		{
			return create_object(v[0], v[1]);
		}

		throw(std::wstring(L"Invoke err"));

	}
};




class MessageLayerPlate
{
public:
	MessageLayerPlate() : target_(nullptr), capture_lock_(false){ mst_ ={}; }

	std::vector<DrawingObject*> objects_;
	DrawingObject* target_;
	ScriptSt mst_;
	bool capture_lock_;

	LRESULT WindowProc(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT ret = 0;
		static CPoint point_prv(0,0);
		switch (message)
		{
			case WM_LBUTTONDOWN:
			{
				if (!capture_lock_)
					::SetCapture(hWnd);

				CPoint point(LOWORD(lParam), HIWORD(lParam));
				point_prv = point;
				target_ = nullptr;

				for (auto& obj : objects_)
				{
					if ( dynamic_cast<IVARIANTButton*>(obj))
					{
						if ( obj->getRect().PtInRect(point))
						{
							target_ = obj;
							CRect rc = obj->getRect();
							rc.OffsetRect(2, 2);
							obj->setRect(rc );
							InvalidateRect(hWnd, NULL, TRUE);
							ret = 1;
							break;
						}						
					}
					else if (dynamic_cast<IVARIANTListbox*>(obj))
					{
						CRect rc;
						auto ls = dynamic_cast<IVARIANTListbox*>(obj);

						auto idx = ls->getItem(point, rc);
						if ( idx > -1 )
						{
							ls->select(idx);
				
							InvalidateRect(hWnd, NULL, TRUE);
							ret = 1;
						}
						else if ( idx == -2 )
						{
							target_ = obj;
							ls->scrollbar(true);

						}
						
					}
				}

			}
			break;
			case WM_LBUTTONUP:
			{
				if (::GetCapture() == hWnd)
				{
					if (dynamic_cast<IVARIANTButton*>(target_) && mst_.result)
					{
						CRect rc = target_->getRect();
						rc.OffsetRect(-2, -2);
						target_->setRect(rc);

						auto btn = dynamic_cast<IVARIANTAbstract*>(target_);

						_variant_t v1(btn);

						VARIANT v = ScriptCall(mst_, L"OnClick", &v1, 1);


						InvalidateRect(hWnd,NULL,TRUE);

					}
					else if (dynamic_cast<IVARIANTListbox*>(target_))
					{
						auto ls = dynamic_cast<IVARIANTListbox*>(target_);
						/*CRect rc;
						auto idx = ls->getItem(point, rc);
						if (idx > -1)
						{
							ls->select(idx);
							InvalidateRect(hWnd, NULL, TRUE);
						}*/
					}

					if (!capture_lock_)
						::ReleaseCapture();

					target_ = nullptr;
					ret = 1;
				}
			}
			break;
			case WM_MOUSEMOVE:
			{
				int a = 0;

				if (::GetCapture() == hWnd && dynamic_cast<IVARIANTListbox*>(target_))
				{
					if (target_ != nullptr)
					{
						CPoint point(LOWORD(lParam), HIWORD(lParam));
						

						auto ls = dynamic_cast<IVARIANTListbox*>(target_);
						ls->scrollbarMove(point.y-point_prv.y);

						InvalidateRect(hWnd, NULL, FALSE);
						
						point_prv = point;

						ret = 1;
					}
				}
			}
			break;
			case WM_KEYDOWN:
			{
				if ( wParam == VK_ESCAPE )
					capture_lock_ = false;

			}
			break;
			
		}
		

		return ret;
	}
	bool Open(CMFCTestView* parent)
	{
		auto mst = MNSCInitilize(this, 1);
		mst_ = mst;

		CComBSTR script;
		auto bl = MNSCReadUtf8(L"script\\init.txt", &script);

		if ( bl )
		{

			_variant_t v1(new IVARIANTApplication(parent), false);

			bl = MNSCParse(mst, script, L"_ap", v1);

			if (bl){
				VARIANT v = ScriptCall(mst, L"OnInit", nullptr, 0);
			}
		}

		return (bl && mst.result);
	}
	void Close()
	{
		for (auto& obj : objects_)
		{
			auto p = dynamic_cast<IVARIANTAbstract*>(obj);
			//p->Release(); <--VariantClearでエラーになる
		}
		MNSCClose(mst_);
	}

};

// CMFCTestView 描画

void CMFCTestView::OnDraw(CDC* pDC)
{

	for (auto& obj : uilayer_->objects_)
	{
		obj->Draw(pDC);
	}

}

BOOL CMFCTestView::PreTranslateMessage(MSG* pMsg)
{	
	if ((WM_MOUSEFIRST <= pMsg->message && pMsg->message <= WM_MOUSELAST)
		|| (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST))
		return (0 != uilayer_->WindowProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam));

	return CView::PreTranslateMessage(pMsg);
}

void CMFCTestView::OnDestroy()
{
	CView::OnDestroy();

	uilayer_->Close();


}




int CMFCTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	uilayer_ = std::make_unique<MessageLayerPlate>();

	if ( !uilayer_->Open(this))
		return -1;

	return 0;
}

VARIANT IVARIANTApplication::create_object(VARIANT typ, VARIANT v)
{
	if (v.vt == VT_UNKNOWN && typ.vt == VT_BSTR)
	{
		CComBSTR objtyp = typ.bstrVal;
		IVARIANTMap* par = dynamic_cast<IVARIANTMap*>(v.punkVal);
		if (par) 
		{
			_variant_t x, y, cx, cy, text;
			if (!par->GetItem(L"x", &x))
				x = 100;
			if (!par->GetItem(L"y", &y))
				y = 100;
			if (!par->GetItem(L"cx", &cx))
				cx = 30;
			if (!par->GetItem(L"cy", &cy))
				cy = 200;
			if (!par->GetItem(L"text", &text))
				text = L"notdef";

			if (objtyp == L"button")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto obj = dynamic_cast<DrawingObject*>(new IVARIANTButton(rc));

				//obj->setRect(rc);

				pview_->uilayer_->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));


				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = dynamic_cast<IVARIANTButton*>(obj);
				//v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"listbox")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto p = new IVARIANTListbox(rc);
				auto obj = dynamic_cast<DrawingObject*>(p);
				pview_->uilayer_->objects_.push_back(obj);

				p->setText(text);


				VARIANT vfunc;
				::VariantInit(&vfunc);

				if (par->GetItem(L"onselect", &vfunc))
					p->func_onselect_.Attach(vfunc);


				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = p;
				//v1.punkVal->AddRef();
				return v1;

			}
			else if (objtyp == L"textbox")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto obj = dynamic_cast<DrawingObject*>(new IVARIANTTextbox(rc));

				//obj->setRect(rc);

				pview_->uilayer_->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));


				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = dynamic_cast<IVARIANTTextbox*>(obj);
				//v1.punkVal->AddRef();
				return v1;
			}
			
		}
	}

	CComVariant ret(0);
	return ret;
}