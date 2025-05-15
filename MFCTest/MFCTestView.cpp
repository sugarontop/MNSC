
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
		if (target_ != nullptr)
		{
			target_->rc_.OffsetRect(-2, -2);

			VARIANT v = MNSCCall(mst_, L"OnClick", nullptr, 0); //prms, 1);
			
			
			
			
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

	VARIANT create_object(VARIANT typ, VARIANT x, VARIANT y, VARIANT txt)
	{
		CRect rc;
		rc.left = x.intVal;
		rc.top = y.intVal;
		rc.right = rc.left + 200;
		rc.bottom = rc.top + 40;

		if (typ.vt == VT_BSTR)
		{
			if (wcscmp(typ.bstrVal, L"button") == 0)
			{
				auto obj = std::make_shared<DrawingObject>(rc);
				pview_->objects_.push_back(obj);

				obj->text_ = txt.bstrVal;



			}
			else if (wcscmp(typ.bstrVal, L"listbox") == 0)
			{
				
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
		if (funcnm == L"create_object" && vcnt > 3)
		{
			return create_object(v[0], v[1], v[2], v[3]);
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


	VARIANT v1;
	::VariantInit(&v1);
	v1.punkVal = new IVARIANTApplication(this);
	v1.vt = VT_UNKNOWN;



	bl = MNSCParse(mst_, script, L"_ap", v1);

	VARIANT v = MNSCCall(mst_, L"OnInit", nullptr, 0); //prms, 1);


	::VariantClear(&v1);

	return 0;
}

void CMFCTestView::OnDestroy()
{
	CView::OnDestroy();

	MNSCClose(mst_);
}
