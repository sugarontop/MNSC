
// MFCEditDoc.cpp : CMFCEditDoc クラスの実装
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "MFCEdit.h"
#endif

#include "MFCEditDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCEditDoc

IMPLEMENT_DYNCREATE(CMFCEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCEditDoc, CDocument)
END_MESSAGE_MAP()


// CMFCEditDoc コンストラクション/デストラクション

CMFCEditDoc::CMFCEditDoc() noexcept
{
	// TODO: この位置に 1 度だけ呼ばれる構築用のコードを追加してください。

}

CMFCEditDoc::~CMFCEditDoc()
{
}

BOOL CMFCEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。

	return TRUE;
}




// CMFCEditDoc のシリアル化

void CMFCEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 格納するコードをここに追加してください。
	}
	else
	{
		// TODO: 読み込むコードをここに追加してください。
	}
}
// CMFCEditDoc の診断

#ifdef _DEBUG
void CMFCEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCEditDoc コマンド
