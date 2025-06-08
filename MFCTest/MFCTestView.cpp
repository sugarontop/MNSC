
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


#include "IVARIANTApplication.h"
#include "MessageLayerPlate.h"

#pragma comment(lib,"MNSC.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCTestView

IMPLEMENT_DYNCREATE(CMFCTestView, CView)

BEGIN_MESSAGE_MAP(CMFCTestView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_MESSAGE(WM_BRADCAST_SET_INIT, &CMFCTestView::OnMyCustomMessage)
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



// CMFCTestView 描画

void CMFCTestView::OnDraw(CDC* pDC)
{
	CFont cf;
	cf.CreatePointFont(110, L"Meiryo UI");
	CFont* old = pDC->SelectObject(&cf);

	for (auto& obj : uilayers_[active_layer_]->objects_)
	{
		obj->Draw(pDC);
	}


	pDC->SelectObject(old);
}

BOOL CMFCTestView::PreTranslateMessage(MSG* pMsg)
{	
	if ((WM_MOUSEFIRST <= pMsg->message && pMsg->message <= WM_MOUSELAST)
		|| (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST))
		return (0 != uilayers_[active_layer_]->WindowProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam));

	return CView::PreTranslateMessage(pMsg);
}

void CMFCTestView::OnDestroy()
{
	CView::OnDestroy();

	for(auto& ui : uilayers_ )
		ui->Close();

	TSFClose(m_hWnd);

}

int CMFCTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	TSFInit(m_hWnd);

	auto ui = std::make_shared<MessageLayerPlate>();// view_id==0

	active_layer_ = 0;
	uilayers_.push_back(ui);


	auto ui2 = std::make_shared<MessageLayerPlate>(); // view_id==1
	uilayers_.push_back(ui2);

	if ( !ui->Open(this, L"script\\init.txt"))
		return -1;

	return 0;
}


void CMFCTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_F1)
	{
		SendMessage( WM_BRADCAST_SET_INIT,0,0);
		
		
		// 画面の切り替え
		active_layer_ = (active_layer_ == 0 ? 1 : 0);
		Invalidate();
		return;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CMFCTestView::OnMyCustomMessage(WPARAM wParam, LPARAM lParam)
{
	uilayers_[active_layer_]->WindowProc(m_hWnd, WM_BRADCAST_SET_INIT, 0,0);
	return 0;
}