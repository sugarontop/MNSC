
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

#pragma comment(lib,"MNSC.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCTestView

IMPLEMENT_DYNCREATE(CMFCTestView, CView)

BEGIN_MESSAGE_MAP(CMFCTestView, CView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
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

// CMFCTestView 描画

void CMFCTestView::OnDraw(CDC* pDC)
{
	CMFCTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	for (auto& obj : objects_)
	{
		obj->Draw(pDC);
	}
	
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

void CMFCTestView::OnLButtonDown(UINT nFlags, CPoint point)
{
	::SetCapture(m_hWnd);


	for (auto& obj : objects_)
	{
		if ( obj->rc_.PtInRect(point))
		{
			target_ = obj;
			CRect rc = obj->rc_;
			obj->rc_.OffsetRect(2, 2);
			InvalidateRect(NULL);
			//UpdateWindow();
			break;
		}
		
	}


	
}

void CMFCTestView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (::GetCapture() == m_hWnd)
	{
		if (target_ != nullptr && mst_.result)
		{
			target_->rc_.OffsetRect(-2, -2);

			_variant_t v1(new IVARIANTButton(target_.get()),false);

			VARIANT v = ScriptCall(mst_, L"OnClick", &v1, 1);
			
			
			InvalidateRect(NULL);

		}

		::ReleaseCapture();

		target_ = nullptr;
	}
	
}

void CMFCTestView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	CView::OnMouseMove(nFlags, point);
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

	VARIANT create_object(VARIANT typ, VARIANT v)
	{
		if (v.vt == VT_UNKNOWN && typ.vt == VT_BSTR)
		{
			CComBSTR objtyp = typ.bstrVal;
			IVARIANTMap* par = (IVARIANTMap*)v.punkVal;
			auto id = par->TypeId();
			if (id == 2)
			{
				_variant_t x,y,cx,cy,text;
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

					auto obj = std::make_shared<DrawingObject>(rc);
					pview_->objects_.push_back(obj);

					obj->text_ = text.bstrVal;
				}			
				else if ( objtyp == L"listbox")
				{
					// テスト



				}


			}




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
		if (funcnm == L"create_object" && vcnt > 0)
		{
			return create_object(v[0], v[1]);
		}
		
		throw(std::wstring(L"Invoke err"));

	}
};

int CMFCTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	mst_ = MNSCInitilize(this, 1);


	CComBSTR script;
	auto bl = MNSCReadUtf8(L"script\\init.txt", &script);

	_variant_t v1(new IVARIANTApplication(this), false);

	bl = MNSCParse(mst_, script, L"_ap", v1);

	VARIANT v = ScriptCall(mst_, L"OnInit", nullptr, 0); //prms, 1);



	return 0;
}

void CMFCTestView::OnDestroy()
{
	CView::OnDestroy();

	MNSCClose(mst_);
}
