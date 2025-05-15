
// MFCTest.h : MFCTest アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CMFCTestApp:
// このクラスの実装については、MFCTest.cpp を参照してください
//

class CMFCTestApp : public CWinApp
{
public:
	CMFCTestApp() noexcept;


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	BOOL  m_bHiColorIcons;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCTestApp theApp;
