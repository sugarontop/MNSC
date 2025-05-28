
// MFCTestDoc.cpp : CMFCTestDoc クラスの実装
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "MFCTest.h"
#endif

#include "MFCTestDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCTestDoc

IMPLEMENT_DYNCREATE(CMFCTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCTestDoc, CDocument)
END_MESSAGE_MAP()


// CMFCTestDoc コンストラクション/デストラクション

CMFCTestDoc::CMFCTestDoc() noexcept
{
	// TODO: この位置に 1 度だけ呼ばれる構築用のコードを追加してください。

}

CMFCTestDoc::~CMFCTestDoc()
{
}

BOOL CMFCTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。

	return TRUE;
}




// CMFCTestDoc のシリアル化

void CMFCTestDoc::Serialize(CArchive& ar)
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

#ifdef SHARED_HANDLERS

//縮小版のサポート
void CMFCTestDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// このコードを変更してドキュメントのデータを描画します
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 検索ハンドラーのサポート
void CMFCTestDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ドキュメントのデータから検索コンテンツを設定します。
	// コンテンツの各部分は ";" で区切る必要があります

	// 例:      strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMFCTestDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMFCTestDoc の診断

#ifdef _DEBUG
void CMFCTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCTestDoc コマンド
#define _SECOND ((__int64) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR)

ULONG UnixTime(int yyyy, int mm, int dd)
{
	if (yyyy < 1970)
	{
		struct tm tm_now;
		__time64_t long_time;
		_time64(&long_time);
		localtime_s(&tm_now, &long_time);

		yyyy = 1900 + tm_now.tm_year;
		mm = 1 + tm_now.tm_mon;
		dd = tm_now.tm_mday;
	}


	const ULONG oneday = 86400; // 86400sec

	SYSTEMTIME st = {};
	SYSTEMTIME st2 = {};

	st.wYear = 1970; st.wMonth = 1; st.wDay = 1;

	st2.wYear = yyyy; st2.wMonth = mm; st2.wDay = dd;

	FILETIME ft, ft2;
	SystemTimeToFileTime(&st, &ft);
	SystemTimeToFileTime(&st2, &ft2);

	ULONGLONG number = (((ULONGLONG)ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	ULONGLONG number2 = (((ULONGLONG)ft2.dwHighDateTime) << 32) + ft2.dwLowDateTime;

	auto distance = (number2 - number) / _DAY;
	auto ret = distance * oneday;


	return (ULONG)ret;

}