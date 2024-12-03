
// MFCEditView.cpp : CMFCEditView クラスの実装
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "MFCEdit.h"
#endif

#include "MFCEditDoc.h"
#include "MFCEditView.h"

#include "mnsc.h"
#include <codecvt>
#include <regex>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FIRSTPAGE 0

#include "mnsc.h"
#pragma comment(lib,"MNSC.lib")

#include <msxml6.h>
#pragma comment( lib, "msxml6")

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSimpleEdit, CEdit)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CSimpleEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_TAB) // Tabキーが押された場合
	{
		// カーソル位置にタブ文字を挿入
		int start, end;
		GetSel(start, end);  // 現在の選択範囲を取得
		ReplaceSel(_T("\t"), TRUE);  // タブ文字を挿入

		return; // イベントを処理済みとして他に伝播しない
	}

	// デフォルトの動作を呼び出す
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
//////////////////////////////////////////////////////////////////////////

std::vector<std::wstring> Split(std::wstring str, std::wstring split_str);

bool WriteUtf8File(const CString& filePath, const CStringW& utf16Content);
bool ReadUtf8File(const CString& filePath, CStringW& utf16Content);


IMPLEMENT_DYNCREATE(CMFCEditView, CFormView)

BEGIN_MESSAGE_MAP(CMFCEditView, CFormView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFormView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CMFCEditView::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_RUN, &CMFCEditView::OnBnClickedBtnRun)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CMFCEditView::OnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCEditView::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CMFCEditView::OnBnClickedBtnClear)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CMB_FONT, &CMFCEditView::OnSelchangeCmbFont)
	ON_EN_KILLFOCUS(IDC_ED_INPUT, &CMFCEditView::OnKillfocusEdInput)
	ON_EN_SETFOCUS(IDC_ED_INPUT, &CMFCEditView::OnSetfocusEdInput)
END_MESSAGE_MAP()

// CMFCEditView コンストラクション/デストラクション

CMFCEditView::CMFCEditView() noexcept
	: CFormView(IDD_MFCEDIT_FORM)
{
	// TODO: 構築コードをここに追加します。


}

CMFCEditView::~CMFCEditView()
{
}

void CMFCEditView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ED_INPUT, m_edit);
	DDX_Control(pDX, IDC_COMBO1, m_cmb);
	DDX_Control(pDX, IDC_CMB_FONT, m_cmb_font);
}

BOOL CMFCEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

void PasteTemplate(CEdit& edit)
{	
	std::wstringstream sm;

	sm << L"\r\nfunc template()\r\n" << L"var a,b,c;\r\n";
	sm << L"begin\r\n\t\r\n\t\r\nend\r\n\r\n";
	
	CString sc;
	edit.GetWindowText(sc);
	sm << (LPCWSTR)sc;

	edit.SetWindowText(sm.str().c_str());
}


BOOL CMFCEditView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Ctrlキーが押されているか確認
		if (GetKeyState(VK_CONTROL) < 0)
		{
			switch (pMsg->wParam)
			{
			case 'C':
				m_edit.Copy();
				return TRUE;
			case 'V':
				m_edit.Paste();
				return TRUE;
			case 'X':
				m_edit.Cut();
				return TRUE;
			case 'Z':
				m_edit.Undo();
				return TRUE;
			case 'W':
				PasteTemplate(m_edit);
				return TRUE;
			case 'S':
				OnBnClickedBtnSave();
				return TRUE;
			}
		}
		else if (GetKeyState(VK_SHIFT) < 0)
		{
			if ( pMsg->wParam == VK_INSERT )
			{
				m_edit.Paste();
				return TRUE;
			}
		}

		int line = (int)::SendMessage(GetDlgItem(IDC_ED_INPUT)->m_hWnd, EM_LINEFROMCHAR, -1, 0);
		CString s; s.Format(L"%d", line+1);
		GetDlgItem(IDC_ED_LINENUMBER)->SetWindowText(s);
	}
	else if (pMsg->message == WM_LBUTTONUP)
	{
		if ( pMsg->hwnd == GetDlgItem(IDC_ED_INPUT)->GetSafeHwnd())
		{
			int line = (int)::SendMessage(GetDlgItem(IDC_ED_INPUT)->m_hWnd, EM_LINEFROMCHAR, -1, 0);
			CString s; s.Format(L"%d", line + 1);
			GetDlgItem(IDC_ED_LINENUMBER)->SetWindowText(s);
		}
	}
	
	return CFormView::PreTranslateMessage(pMsg);
}


void CMFCEditView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	m_wrk_dir = L".\\";
	m_font_height = 20;


	int tabStops = 20;
	m_edit.SendMessage(EM_SETTABSTOPS, 1, (LPARAM)&tabStops);
	
	GetDlgItem(IDC_ED_INPUT)->EnableWindow();
	GetDlgItem(IDC_ED_OUTPUT)->SetWindowText(L"output view");


	m_cmb_font.AddString(L"メイリオ");
	m_cmb_font.AddString(L"MS 明朝");
	m_cmb_font.AddString(L"Fira Code");

	
	////////////////////

	WIN32_FIND_DATA dd={};	
	auto hf = FindFirstFile(L"script\\*.txt", &dd);
	BOOL bl = (hf != INVALID_HANDLE_VALUE);
	while(bl)
	{
		m_cmb.AddString(dd.cFileName);
		bl = FindNextFile(hf, &dd);
	}

	GetDlgItem(IDC_COMBO1)->SendMessage(CB_SETCURSEL, (WPARAM)FIRSTPAGE, (LPARAM)0);


	////////////////////
	CString init_file = m_wrk_dir + L"init.bin";
	CFile file;
	if (file.Open(init_file, CFile::modeRead | CFile::typeBinary)) 
	{		
		InitFile_Archive(file, false);
		file.Close();
	}
	
	SetFont(0);


	OnSelchangeCombo1();
}

// CMFCEditView の診断



void CMFCEditView::InitFile_Archive(CFile& cf, bool bWrite)
{
	int idx;
	if (bWrite)
	{
		idx = m_cmb.GetCurSel();
		cf.Write(&idx, sizeof(int));
		idx = m_cmb_font.GetCurSel();
		cf.Write(&idx, sizeof(int));
		cf.Write(&m_font_height, sizeof(int));

	}
	else
	{
		cf.Read(&idx, sizeof(int));
		m_cmb.SetCurSel(idx);
		cf.Read(&idx, sizeof(int));
		m_cmb_font.SetCurSel(idx);
		cf.Read(&m_font_height, sizeof(int));
	}
}
void CMFCEditView::SetFont(int type)
{
	if (type > 0)
		m_font_height = max(12, m_font_height - 2);
	else if (type < 0)
		m_font_height = min(36, m_font_height + 2);

	CFont* pf = GetDlgItem(IDC_ED_INPUT)->GetFont();
	LOGFONT lg = {};
	pf->GetLogFont(&lg);

	CFont cfn;
	lg.lfHeight = m_font_height;
	cfn.CreateFontIndirect(&lg);

	int id[] = { IDC_ED_INPUT,IDC_ED_OUTPUT,IDC_ED_OUTPUT3,IDC_ED_LINENUMBER,IDC_ED_FILENAME };
	for(auto& i : id)
		GetDlgItem(i)->SetFont(&cfn);
}

#include <queue>
class CVARIANTTool : public IVARIANTAbstract
{
	public :
		CVARIANTTool(){};
		~CVARIANTTool(){};
		virtual void Clear(){}
		virtual int TypeId() { return 1001; }
	protected:		
		VARIANT inetGet(const VARIANT url)
		{			
			HRESULT hr;
			CComPtr<IXMLHTTPRequest> req;
			_bstr_t bs, headers;
						
			hr = req.CoCreateInstance(CLSID_XMLHTTP60);
			hr = req->open(
				 _bstr_t("GET"),
				 url.bstrVal,
				 _variant_t(VARIANT_FALSE),
				 _variant_t(),
				 _variant_t());
			hr = req->send(_variant_t());
			
			long status;
			hr = req->get_status(&status);//200 : succuss
			
			VARIANT ret = MNSCCreateMap();
			IVARIANTMap* map = dynamic_cast<IVARIANTMap*>(ret.punkVal);

			if (status == 200)
			{				
				req->get_responseText(bs.GetAddress());

				req->getAllResponseHeaders(headers.GetAddress());

				std::wstring cheaders = headers;
				std::wstring split = L"\r\n";

				std::vector<std::wstring> ar = Split(cheaders, split);

				for(auto& it : ar)
				{
					auto xar = Split(it,L":");

					if ( xar.size() == 2 )
					{
						_variant_t v(xar[1].c_str());
						map->SetItem(xar[0], v.Detach() );
					}
				}

				map->SetItem(L"result", _variant_t(status).Detach());
				map->SetItem(L"json", _variant_t(bs).Detach());
				

				//FILE* pf;
				//std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				//CStringA strA((LPCWSTR)bs);
				//::fopen_s(&pf, "test.json", "w");
				//std::string utf8 = convert.to_bytes(bs);
				//::fwrite((LPSTR)(LPCSTR)utf8.c_str(), 1, utf8.length(), pf);
				//fclose(pf);

				return ret;
			}
			 else
				map->SetItem(L"result", _variant_t(status).Detach());

			return ret;
		}

		VARIANT Value(const VARIANT obj, const VARIANT query)
		{
			if (obj.vt != VT_UNKNOWN || query.vt != VT_BSTR)
				THROW(L"Query err");

			LPCWSTR p = query.bstrVal;

			struct st
			{
				st(int a) :typ(a), keyN(0) {}
				int typ;
				std::wstring keyS;
				UINT keyN;

			};

			std::queue< std::shared_ptr<st>> qu;
			while (*p)
			{
				while (*p <= ' ') p++;

				if (*p == '"')
				{
					auto x = std::make_shared<st>(2);
					qu.push(x);

					p++;
					std::wstringstream sm;
					while (*p != '"')
					{
						sm << *p;
						p++;
					}
					x->keyS = sm.str();
				}
				else if ('0' <= *p && *p <= '9')
				{
					auto x = std::make_shared<st>(1);
					qu.push(x);

					std::wstringstream sm;
					while ('0' <= *p && *p <= '9')
					{
						sm << *p++;
					}
					int idx = _wtoi(sm.str().c_str());
					x->keyN = idx;
					p--;
				}

				p++;
			}

			IUnknown* target = obj.punkVal;
			_variant_t ret;
			while (1)
			{
				_variant_t v;

				auto st = qu.front();
				if (st->typ == 1)
				{
					auto ar = dynamic_cast<IVARIANTArray*>(target);
					if (ar == nullptr)
						goto err;

					auto idx = st->keyN;

					if (idx < ar->Count())
						ar->Get(idx, &v);
					else
						goto err;
				}
				else if (st->typ == 2)
				{
					auto map = dynamic_cast<IVARIANTMap*>(target);
					if (map == nullptr)
						goto err;

					if (!map->GetItem(st->keyS, &v))
						goto err;
				}

				qu.pop();

				if (v.vt != VT_UNKNOWN || qu.empty())
				{
					ret = v;
					break;
				}

				target = v.punkVal;
			}
		err:
			return ret.Detach();
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
		virtual VARIANT Invoke(std::wstring funcnm, VARIANT* prm, int vcnt) override
		{
			if (vcnt == 1 && funcnm == L"inetGet")
			{				
				VARIANT url = prm[0];

				return inetGet(url);
			}
			else if (vcnt == 2 && funcnm == L"value")
			{
				return Value(prm[0],prm[1]);
			}
			

		}
		
};


class IVARIANTApplication : public IVARIANTAbstract
{
public:
	IVARIANTApplication(HWND hWnd):parent_(hWnd) {};
public:
	HWND parent_;
public :
	virtual void Clear(){} 
	virtual int TypeId() { return 1000; }
	VARIANT Test(const VARIANT a, const VARIANT b)
	{
		std::wstring a1 = a.bstrVal;
		std::wstring b1 = b.bstrVal;

		a1 = a1 + b1;
		std::wstring str = a1;

		VARIANT v;
		::VariantInit(&v);
		
		v.bstrVal = ::SysAllocString(str.c_str());
		v.vt = VT_BSTR;

		HWND output = ::GetDlgItem(parent_,IDC_ED_OUTPUT3);

		SetWindowText(output, str.c_str());
		
		return v;
	}
	VARIANT Tool()
	{
		VARIANT v;
		::VariantInit(&v);
		
		v.punkVal = new CVARIANTTool();
		v.vt = VT_UNKNOWN;
		
		return v;
	}

	VARIANT SetStatusText(VARIANT vText)
	{		
		HWND output = ::GetDlgItem(parent_, IDC_ED_OUTPUT3);

		if ( vText.vt == VT_BSTR)
			SetWindowText(output, vText.bstrVal);
		
		return _variant_t(L"success").Detach();
	}

public :
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
	virtual VARIANT Invoke(std::wstring funcnm, VARIANT* v, int vcnt) override
	{		
		if ( vcnt == 2 && funcnm == L"test")
		{
			return Test(v[0],v[1]);
		}
		else if ( funcnm == L"tool" )
		{
			return Tool();			
		}
		else if (funcnm == L"status")
		{
			return SetStatusText(v[0]);
		}
		
		
		throw(std::wstring(L"Invoke err"));
		
	}
};

void CMFCEditView::OnBnClickedBtnRun()
{
	// how to use MNSC

	CWaitCursor _w;

	// change stt::wcout
	std::wstringstream sm;
	std::wcout.rdbuf(sm.rdbuf());

	// 1 create IUnknow object on application side.
	VARIANT v1;
	::VariantInit(&v1);
	v1.punkVal = new IVARIANTApplication(m_hWnd);
	v1.vt = VT_UNKNOWN;

	// 2 script initialize
	ScriptSt sc = MNSCInitilize(this, 1);

	try 
	{	
		CString script;
		GetDlgItem(IDC_ED_INPUT)->GetWindowText(script);

		// 3 script parse
		if (MNSCParse(sc, script, L"_ap", v1))
		{
			VARIANT prms[1];
			_variant_t prm1 = L"Hello world";
			prms[0] = prm1.Detach();

			// 4 script execute
			VARIANT v = MNSCCall(sc,L"main", prms, 1 );

			::VariantClear(&v);


			// std::wcout string to window
			GetDlgItem(IDC_ED_OUTPUT)->SetWindowText(sm.str().c_str());
		}
	}
	catch(std::wstring x)
	{
		GetDlgItem(IDC_ED_OUTPUT)->SetWindowText(x.c_str());
	}

	// 5 script close
	MNSCClose(sc);
}

void CMFCEditView::OnBnClickedBtnSave()
{	
	if ( GetDlgItem(IDC_ED_INPUT)->IsWindowEnabled())
	{
		CString s,fnm;
		GetDlgItem(IDC_ED_INPUT)->GetWindowText(s);

		GetDlgItem(IDC_ED_FILENAME)->GetWindowText(fnm);

		CString fnm2 = L"script\\" + fnm;
		WriteUtf8File(fnm2, s );
	}
}

void CMFCEditView::OnSelchangeCombo1()
{
	CString fnm;
	m_cmb.GetWindowText(fnm);

	CString fnm2 = L"script\\";
	fnm2 += fnm;

	CString s;
	bool bl = ReadUtf8File(fnm2, s);
	
	if ( bl )
	{
		GetDlgItem(IDC_ED_INPUT)->SetWindowText(s);

		if ( fnm.Left(4) != L"test" )
		{
			GetDlgItem(IDC_BTN_SAVE)->ShowWindow(0); 
			GetDlgItem(IDC_ED_FILENAME)->ShowWindow(0);
			GetDlgItem(IDC_ED_FILENAME)->SetWindowText(fnm);
		}	
		else
		{
			GetDlgItem(IDC_BTN_SAVE)->ShowWindow(1); 
			GetDlgItem(IDC_ED_FILENAME)->ShowWindow(1);
			GetDlgItem(IDC_ED_FILENAME)->SetWindowText(fnm);
		}
	}
}


void CMFCEditView::OnBnClickedButton2()
{
	if (0x8000 & GetKeyState(VK_CONTROL))
		SetFont(-1);
	else
		SetFont(1);
}


void CMFCEditView::OnBnClickedBtnClear()
{
	int id [] = { IDC_ED_OUTPUT,IDC_ED_OUTPUT3,IDC_ED_INPUT,IDC_ED_LINENUMBER, IDC_ED_FILENAME };
	for(auto i : id)
		GetDlgItem(i)->SetWindowText(L"");
}

void CMFCEditView::OnDestroy()
{
	CFormView::OnDestroy();

	CString init_file = m_wrk_dir + L"init.bin";

	CFile file;
	if (file.Open(init_file, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		InitFile_Archive(file, true);
		file.Close();
	}
}

void CMFCEditView::OnSelchangeCmbFont()
{
	CString fontnm;
	m_cmb_font.GetWindowText(fontnm);
	int id = m_cmb_font.GetCurSel();
	
	CFont cfn;
	LOGFONT lg = {};
	lg.lfHeight = m_font_height;
	lg.lfCharSet = (id < 2 ? SHIFTJIS_CHARSET : 0);

	lstrcpy(lg.lfFaceName, fontnm);
	cfn.CreateFontIndirect(&lg);

	int id2[] = { IDC_ED_OUTPUT,IDC_ED_OUTPUT3,IDC_ED_INPUT,IDC_ED_LINENUMBER, IDC_ED_FILENAME };
	for (auto i : id2)
		GetDlgItem(i)->SetFont(&cfn);
}


void CMFCEditView::OnKillfocusEdInput()
{
	GetDlgItem(IDC_ED_LINENUMBER)->SetWindowText(L"");
}


void CMFCEditView::OnSetfocusEdInput()
{
	int line = (int)::SendMessage(GetDlgItem(IDC_ED_INPUT)->m_hWnd, EM_LINEFROMCHAR, -1, 0);
	CString s; 
	s.Format(L"%d", line + 1);
	GetDlgItem(IDC_ED_LINENUMBER)->SetWindowText(s);
}
bool WriteUtf8File(const CString& filePath, const CStringW& utf16Content)
{
	// convert UTF-16 to UTF-8
	int len = WideCharToMultiByte(CP_UTF8, 0, utf16Content, utf16Content.GetLength(), nullptr, 0, nullptr, nullptr);
	std::vector<char> v(len);
	WideCharToMultiByte(CP_UTF8, 0, utf16Content, utf16Content.GetLength(), &v[0], len, nullptr, nullptr);

	CStringA afile(filePath);

	FILE* f;
	if (0 == ::fopen_s(&f, afile, "w"))
	{
		for (int i = 0; i < v.size(); i++)
		{
			char c = v[i];
			if (c != '\r')
				::fwrite(&c, 1, 1, f);
		}
		::fclose(f);
		return true;
	}
	
	return false;	
}
bool ReadUtf8File(const CString& filePath, CStringW& utf16Content)
{
	CStdioFile file;
	if (!file.Open(filePath, CFile::modeRead | CFile::typeBinary))
		return false;

	bool ret = true;

	UINT fileSize = (UINT)file.GetLength();
	BYTE* buffer = new BYTE[(size_t)fileSize + 1];
	file.Read(buffer, fileSize);
	buffer[fileSize] = '\0';

	// convert UTF-8 to UTF-16
	int utf16Length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer, -1, nullptr, 0);
	if (utf16Length > 0) {
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer, -1, utf16Content.GetBuffer(utf16Length), utf16Length);
		utf16Content.ReleaseBuffer();
	}
	else
		ret = false;

	delete[] buffer;
	file.Close();

	return ret;
}

std::vector<std::wstring> Split(std::wstring str, std::wstring split_str)
{
	std::vector<std::wstring> ar;
	std::wregex re(split_str);
	std::wsregex_token_iterator it(str.begin(), str.end(), re, -1);
	std::wsregex_token_iterator end;

	while (it != end) {
		auto& item = *it++;		
		ar.push_back(item.str());
	}	
	return ar;
}