
// MFCTestView.h : CMFCTestView クラスのインターフェイス
//

#pragma once
#include "mnsc.h"

class DrawingObject;
class CMFCTestView : public CView
{
protected: // シリアル化からのみ作成します。
	CMFCTestView() noexcept;
	DECLARE_DYNCREATE(CMFCTestView)

// 属性
public:
	CMFCTestDoc* GetDocument() const;

	std::vector<std::shared_ptr<DrawingObject>> objects_;

	std::shared_ptr<DrawingObject> target_;

// 操作
public:

// オーバーライド
public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画するためにオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 実装
public:
	virtual ~CMFCTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	ScriptSt mst_;

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // MFCTestView.cpp のデバッグ バージョン
inline CMFCTestDoc* CMFCTestView::GetDocument() const
   { return reinterpret_cast<CMFCTestDoc*>(m_pDocument); }
#endif

