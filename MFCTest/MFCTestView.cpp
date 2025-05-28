
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
#include "x.h"
#include "x2.h"
#include "x3.h"
#include "x4.h"
#include "x5.h"
#include "x6.h"
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

ULONG UnixTime(int yyyy, int mm, int dd);

class IVARIANTApplication : public IVARIANTAbstract
{
public:
	IVARIANTApplication(CMFCTestView* pview) :pview_(pview) {};
public:
	HWND parent_;
public:
	virtual void Clear() {}
	virtual int TypeId()  const { return 1000; }

	CMFCTestView* pview_;

	VARIANT create_object(VARIANT vid,VARIANT typ, VARIANT v);

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
		if (funcnm == L"create_object" && vcnt > 2)
		{
			return create_object(v[0], v[1], v[2]);
		}
		else if (funcnm == L"unixtime" && vcnt > 2)
		{
			VARIANT ret;
			ret.llVal = UnixTime(v[0].intVal, v[1].intVal, v[2].intVal);
			ret.vt = VT_I8;

			return ret;
		}

		throw(std::wstring(L"Invoke err"));

	}
};




class MessageLayerPlate
{
public:
	MessageLayerPlate() : target_(nullptr){ mst_ ={}; }

	std::vector<DrawingObject*> objects_;
	DrawingObject* target_;
	ScriptSt mst_;

	LRESULT WindowProc(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT ret = 0;
		static CPoint point_prv(0,0);

		static DrawingObject* captured_obj = nullptr;

		if (captured_obj)
		{
			auto ls = dynamic_cast<IVARIANTDropdownList*>(captured_obj);
			if ( ls )
				if ( DrowdownListBoxWindowProc(ls, hWnd, message, wParam, lParam, ret))
				{
					captured_obj = nullptr;
					return ret;
				}


			auto txt = dynamic_cast<IVARIANTTextbox*>(captured_obj);
			if (txt)
				if (TextboxWindowProc(txt, hWnd, message, wParam, lParam, ret))
				{
					captured_obj = nullptr;
					return ret;
				}

		}
		else
		{
			switch (message)
			{
				case WM_LBUTTONDOWN:
				{
					if (captured_obj==nullptr)
						::SetCapture(hWnd);

					CPoint point(LOWORD(lParam), HIWORD(lParam));
					point_prv = point;
					target_ = nullptr;

					for (auto& obj : objects_)
					{
						if ( dynamic_cast<IVARIANTButton*>(obj))
						{
							if ( obj->getRect().PtInRect(point))
							{
								target_ = obj;
								CRect rc = obj->getRect();
								rc.OffsetRect(2, 2);
								obj->setRect(rc );
								InvalidateRect(hWnd, NULL, TRUE);
								ret = 1;
								break;
							}						
						}
						else if (dynamic_cast<IVARIANTListbox*>(obj))
						{
							if (obj->getRect().PtInRect(point))
							{
								auto ls = dynamic_cast<IVARIANTListbox*>(obj);

								int md = ls->SelectRow(point);

								if ( md == 1 )
								{
									InvalidateRect(hWnd, NULL, FALSE);
									ret = 1;
								}
								else if ( md == 2 )
								{
									target_ = ls;
									ret = 1;
								}

								target_ = ls;
								return ret;
							}
					
						}
						else if (dynamic_cast<IVARIANTDropdownList*>(obj))
						{							
							if (obj->getRect().PtInRect(point))
							{
								auto ls = dynamic_cast<IVARIANTDropdownList*>(obj);

								if ( ls->BtnClick(point))
								{
									ls->ShowDlgListbox(true);
									captured_obj = ls;

									InvalidateRect(hWnd, NULL, TRUE);
									ret = 1;
									return ret;
								}
							}
						}
						else if (dynamic_cast<IVARIANTTextbox*>(obj))
						{
							if (obj->getRect().PtInRect(point) )
							{							
								auto txt = dynamic_cast<IVARIANTTextbox*>(obj); 
								
								if (txt->ReadOnly() )return ret;

								txt->SetFocus(hWnd, point);

								captured_obj = txt;

								InvalidateRect(hWnd, NULL, TRUE);
								ret = 1;
								return ret;
							}
						}
					}

				}
				break;
				case WM_LBUTTONUP:
				{
					if (::GetCapture() == hWnd)
					{
						CPoint point(LOWORD(lParam), HIWORD(lParam));

						if (dynamic_cast<IVARIANTButton*>(target_) && mst_.result)
						{
							CRect rc = target_->getRect();
							rc.OffsetRect(-2, -2);
							target_->setRect(rc);

							if (target_->getRect().PtInRect(point))
							{
								auto btn = dynamic_cast<IVARIANTAbstract*>(target_);

								_variant_t v1(btn);

								VARIANT v = ScriptCall(mst_, L"OnClick", &v1, 1);
							}

							InvalidateRect(hWnd,NULL,TRUE);
						}
						else if (dynamic_cast<IVARIANTListbox*>(target_))
						{
							auto ls = dynamic_cast<IVARIANTListbox*>(target_);

							CPoint point(LOWORD(lParam), HIWORD(lParam));
							if (1 == ls->SelectRow(point, true))
							{
								InvalidateRect(hWnd, NULL, FALSE);
							}
						}

						if (captured_obj == nullptr)
							::ReleaseCapture();

						target_ = nullptr;
						ret = 1;
					}
				}
				break;
				case WM_MOUSEMOVE:
				{
					if (::GetCapture() == hWnd && dynamic_cast<IVARIANTListbox*>(target_))
					{
						if (target_ != nullptr)
						{
							auto ls = dynamic_cast<IVARIANTListbox*>(target_);
						
							CPoint point(LOWORD(lParam), HIWORD(lParam));
							int offy = point.y - point_prv.y;

							if (2 == ls->SelectRow(point) && GetAsyncKeyState(VK_LBUTTON) & 0x8000)
							{
								ls->ScrollbarYoff(offy);
								InvalidateRect(hWnd, NULL, FALSE);
							}

							point_prv = point;
							ret = 1;
						}
					}
				}
				break;
			
			
			}
		}	

		return ret;
	}
	bool Open(CMFCTestView* parent, LPCWSTR script_file)
	{
		auto mst = MNSCInitilize(this, 1);
		mst_ = mst;

		CComBSTR script;
		auto bl = MNSCReadUtf8(script_file, &script);

		if ( bl )
		{
			_variant_t v1(new IVARIANTApplication(parent), false);

			bl = MNSCParse(mst, script, L"_ap", v1);

			if (bl)
			{
				_variant_t id(0);
				VARIANT v = ScriptCall(mst, L"OnInit", &id, 1 );
				::VariantClear(&v);

				id = 1;
				v = ScriptCall(mst, L"OnInit2", &id, 1);
				::VariantClear(&v);
			}
		}

		return (bl && mst.result);
	}
	void Close()
	{		
		for (auto& obj : objects_)
		{
			dynamic_cast<IUnknown*>(obj)->Release();
		}
		MNSCClose(mst_);
	}

	bool DrowdownListBoxWindowProc(IVARIANTDropdownList* ls, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& ret)
	{
		ret=0;
		static CPoint point_prv(0, 0);
		switch (message)
		{
			case WM_LBUTTONDOWN:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));

				int md = ls->SelectRow(point);

				point_prv = point;
			}
			break;
			case WM_MOUSEMOVE:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));
				int offy = point.y - point_prv.y;

				if (2 == ls->SelectRow(point) && GetAsyncKeyState(VK_LBUTTON) & 0x8000)
				{
					ls->ScrollbarYoff(offy);
					InvalidateRect(hWnd, NULL, FALSE);
				}

				point_prv = point;

			}
			break;
			case WM_LBUTTONUP:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));

				int md = ls->SelectRow(point, true);

				if (1 == md || 0 == md)
				{
					ls->ShowDlgListbox(false);
					//captured_obj = nullptr;
					InvalidateRect(hWnd, NULL, TRUE);
					::ReleaseCapture();
					return true;
				}
			}
			break;
			case WM_KEYDOWN:
			{
				if (wParam == VK_ESCAPE)
				{
					ls->ShowDlgListbox(false);
					InvalidateRect(hWnd, NULL, TRUE);
					//captured_obj = nullptr;
					return true;
				}
			}
			break;
		}

		return false;

	}
	bool TextboxWindowProc(IVARIANTTextbox* txt, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& ret)
	{
		ret = 0;
		static CPoint point_prv(0, 0);
		switch (message)
		{
			case WM_LBUTTONDOWN:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));

				if (!txt->getRect().PtInRect(point))
				{
					txt->ReleaseFocus();
					return true;
				}
				else
					txt->SetCaret(point);

				point_prv = point;
			}
			break;
			case WM_MOUSEMOVE:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));
				int offy = point.y - point_prv.y;

			

				point_prv = point;

			}
			break;
			case WM_LBUTTONUP:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));

				if (!txt->getRect().PtInRect(point))
				{
					txt->ReleaseFocus();
					ret = 1;
					return true;
				}				
			}
			break;
			case WM_KEYDOWN:
			{
				if (txt->ReadOnly()) return false;

				auto rc = txt->getRect();
				InvalidateRect(hWnd, rc, FALSE);

				if (wParam == VK_ESCAPE)
				{
					txt->ReleaseFocus();
					ret = 1;
					return true;
				}
				else if ( wParam == VK_LEFT)
				{
					txt->MoveCaret(-1);
					ret=1;
				}
				else if (wParam == VK_RIGHT)
				{
					txt->MoveCaret(1);
					ret = 1;
				}
				else if (wParam == VK_HOME)
				{
					txt->MoveCaret2(false);
					ret = 1;
				}
				else if (wParam == VK_END)
				{
					txt->MoveCaret2(true);
					ret = 1;
				}
				else if (wParam == VK_DELETE)
				{
					txt->DeleteChar(true);
					ret = 1;
				}
				else if (wParam == VK_BACK)
				{
					txt->DeleteChar(false);
					ret = 1;
				}
			}
			break;
			case WM_CHAR:
			{
				if ( txt->ReadOnly()) return false;

				WCHAR ch = (WCHAR)wParam;
				txt->AddChar(ch);

				auto rc = txt->getRect();
				InvalidateRect(hWnd,rc, FALSE);
			}
			break;
			
		}

		return false;

	}

};

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


}




int CMFCTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	auto ui = std::make_shared<MessageLayerPlate>();// view_id==0

	active_layer_ = 0;
	uilayers_.push_back(ui);


	auto ui2 = std::make_shared<MessageLayerPlate>(); // view_id==1
	uilayers_.push_back(ui2);

	if ( !ui->Open(this, L"script\\init.txt"))
		return -1;

	

	return 0;
}

VARIANT IVARIANTApplication::create_object(VARIANT vid, VARIANT typ, VARIANT v)
{
	if (v.vt == VT_UNKNOWN && typ.vt == VT_BSTR)
	{
		int layer_idx = vid.intVal;
		CComBSTR objtyp = typ.bstrVal;
		IVARIANTMap* par = dynamic_cast<IVARIANTMap*>(v.punkVal);
		if (par) 
		{
			_variant_t x, y, cx, cy, text,brd,readonly;
			if (!par->GetItem(L"x", &x))
				x = 100;
			if (!par->GetItem(L"y", &y))
				y = 100;
			if (!par->GetItem(L"cx", &cx))
				cx = 30;
			if (!par->GetItem(L"cy", &cy))
				cy = 200;
			if (!par->GetItem(L"text", &text))
				text = L"notdef";
			if (!par->GetItem(L"border", &brd))
				brd = 0;
			if (!par->GetItem(L"readonly", &readonly))
				readonly = FALSE;

			if (objtyp == L"button")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);
				auto obj = dynamic_cast<DrawingObject*>(new IVARIANTButton(rc));
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));


				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = dynamic_cast<IVARIANTButton*>(obj);
				v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"listbox")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto p = new IVARIANTListbox(rc);
				auto obj = dynamic_cast<DrawingObject*>(p);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				p->setText(text);


				VARIANT vfunc;
				::VariantInit(&vfunc);

				if (par->GetItem(L"onselect", &vfunc))
					p->func_onselect_.Attach(vfunc);


				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = p;
				v1.punkVal->AddRef();
				return v1;

			}
			else if (objtyp == L"textbox")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto txt = new IVARIANTTextbox(rc);
				auto obj = dynamic_cast<DrawingObject*>(txt);
				_variant_t multiline = false;

				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));

				txt->setProperty(brd, readonly, multiline);

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = txt;
				v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"dropdownlist")
			{

				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto p = new IVARIANTDropdownList(rc);
				auto obj = dynamic_cast<DrawingObject*>(p);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);
				
				p->setText(text);
				
				VARIANT vfunc;
				::VariantInit(&vfunc);

				if (par->GetItem(L"onselect", &vfunc))
					p->func_onselect_.Attach(vfunc);

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = dynamic_cast<IVARIANTDropdownList*>(obj);
				v1.punkVal->AddRef();
				return v1;


			}
			else if (objtyp == L"static")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto txt = new IVARIANTStatic(rc);
				auto obj = dynamic_cast<DrawingObject*>(txt);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = txt;
				v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"canvas")
			{
				// テスト
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto txt = new IVARIANTCanvas(rc);
				auto obj = dynamic_cast<DrawingObject*>(txt);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = txt;
				v1.punkVal->AddRef();
				return v1;
				}

			
		}
	}

	_variant_t ret(0);
	return ret;
}
void CMFCTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_F1)
	{
		// 画面の切り替え
		active_layer_ = (active_layer_ == 0 ? 1 : 0);
		Invalidate();
		return;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
