
// MFCEditView.h : CMFCEditView クラスのインターフェイス
//

#pragma once

class CSimpleEdit : public CEdit
{
protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
public:
	
};
class CMFCEditView : public CFormView
{
protected: // シリアル化からのみ作成します。
	CMFCEditView() noexcept;
	DECLARE_DYNCREATE(CMFCEditView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_MFCEDIT_FORM };
#endif

// 属性
public:
	CMFCEditDoc* GetDocument() const;

// 操作
public:
	void InitFile_Archive(CFile& cf, bool bWrite);
	void SetFont(int type);

// オーバーライド
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void OnInitialUpdate(); // 構築後に初めて呼び出されます。	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
// 実装
public:
	virtual ~CMFCEditView();

protected:

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	CSimpleEdit		m_edit;
	CComboBox		m_cmb;
	CString			m_wrk_dir;
	int				m_font_height;
	CComboBox		m_cmb_font;

	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnRun();	
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeCmbFont();
	afx_msg void OnKillfocusEdInput();
	afx_msg void OnSetfocusEdInput();
};



