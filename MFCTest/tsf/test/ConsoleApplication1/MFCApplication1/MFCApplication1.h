
// MFCApplication1.h : MFCApplication1 アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CMFCApplication1App:
// このクラスの実装については、MFCApplication1.cpp を参照してください
//

class CMFCApplication1App : public CWinApp
{
public:
	CMFCApplication1App() noexcept;


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCApplication1App theApp;
