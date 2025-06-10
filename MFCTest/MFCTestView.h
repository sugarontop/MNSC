
// MFCTestView.h : CMFCTestView クラスのインターフェイス
//

#pragma once
#include "mnsc.h"

class DrawingObject;
class MessageLayerPlate;




class CMFCTestView : public CView
{
protected: // シリアル化からのみ作成します。
	CMFCTestView() noexcept;
	DECLARE_DYNCREATE(CMFCTestView)

// 属性
public:
	CMFCTestDoc* GetDocument() const;

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

public:
	ScriptSt mst_;

	int active_layer_;
	std::vector<std::shared_ptr< MessageLayerPlate>> uilayers_;
	//std::unique_ptr<MessageLayerPlate> uilayer_;

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnMyCustomMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // MFCTestView.cpp のデバッグ バージョン
inline CMFCTestDoc* CMFCTestView::GetDocument() const
   { return reinterpret_cast<CMFCTestDoc*>(m_pDocument); }
#endif

