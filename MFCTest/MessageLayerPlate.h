#pragma once

#include "mnsc.h"


#include "tsf\D2DMisc.h"
#include "x.h"
#include "x2.h"
#include "x3.h"
#include "x4.h"
#include "x5.h"
#include "x6.h"


VARIANT ScriptCall(ScriptSt& st, LPCWSTR funcnm, VARIANT* prms, int pmcnt);
extern TSFIsland tsf_;

#define LOCK_KEEP false
#define UNLOCK_KEEP true

class MessageLayerPlate
{
public:
	MessageLayerPlate() : target_(nullptr) { mst_ = {}; }

	std::vector<DrawingObject*> objects_;
	DrawingObject* target_;
	ScriptSt mst_;
	CPoint point_prv_;

	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT ret = 0;

		static DrawingObject* captured_obj = nullptr;

		if (captured_obj)
		{
			auto ddls = dynamic_cast<IVARIANTDropdownList*>(captured_obj);
			auto txt = dynamic_cast<IVARIANTTextbox*>(captured_obj);
			auto btn = dynamic_cast<IVARIANTButton*>(captured_obj);
			auto ls = dynamic_cast<IVARIANTListbox*>(captured_obj);


			if (ddls)
				ret = DrowdownListBoxWindowProc(ddls, hWnd, message, wParam, lParam);
			else if (txt)
			{
				ret = TextboxWindowProc(txt, hWnd, message, wParam, lParam);

				if ( ret == UNLOCK_KEEP )
				{
					txt->SetFocus(false);

				}
			}
			else if (btn)
				ret = ButtonWindowProc(btn, hWnd, message, wParam, lParam);
			else if (ls)
				ret = ListboxWindowProc(ls, hWnd, message, wParam, lParam);

			if (ret == UNLOCK_KEEP)
			{
				captured_obj = nullptr;
				InvalidateRect(hWnd, NULL, FALSE);
				::ReleaseCapture();

				if (message == WM_LBUTTONDOWN)
					goto block1;

				return UNLOCK_KEEP;
			}
			else
				return LOCK_KEEP;
		}
		
		
block1:
	
		switch (message)
		{
			case WM_LBUTTONDOWN:
			{
				CPoint point(LOWORD(lParam), HIWORD(lParam));
				point_prv_ = point;
				target_ = nullptr;

				for (auto& obj : objects_)
				{
					if (dynamic_cast<IVARIANTButton*>(obj))
					{
						if (obj->getRect().PtInRect(point) && dynamic_cast<IVARIANTButton*>(obj)->enable_ == VARIANT_TRUE)
						{
							target_ = obj;
							captured_obj = obj;
							CRect rc = obj->getRect();
							rc.OffsetRect(2, 2);
							obj->setRect(rc);
							InvalidateRect(hWnd, NULL, FALSE);
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

							if (md == 1)
							{
								InvalidateRect(hWnd, NULL, FALSE);
								ret = 1;
							}
							else if (md == 2)
							{
								target_ = ls;
								ret = 1;
							}
							captured_obj = ls;
							target_ = ls;
							break;
						}

					}
					else if (dynamic_cast<IVARIANTDropdownList*>(obj))
					{
						if (obj->getRect().PtInRect(point))
						{
							auto ls = dynamic_cast<IVARIANTDropdownList*>(obj);

							if (ls->BtnClick(point))
							{
								ls->ShowDlgListbox(true);
								captured_obj = ls;

								InvalidateRect(hWnd, NULL, FALSE);
								ret = 1;
								break;
							}
						}
					}
					else if (dynamic_cast<IVARIANTTextbox*>(obj))
					{
						if (obj->getRect().PtInRect(point))
						{
							auto txt = dynamic_cast<IVARIANTTextbox*>(obj);

							//if (txt->ReadOnly() )return ret;


							txt->SetFocus(true);

							captured_obj = txt;
							target_ = txt;

							InvalidateRect(hWnd, NULL, FALSE);
							ret = 1;
							break;
						}
					}
				}

				if (ret == 1 && captured_obj)
					if (::GetCapture() != hWnd)
						::SetCapture(hWnd);

			}
			break;							
		}
	

		return ret;
	}
	bool Open(CMFCTestView* parent, LPCWSTR script_file)
	{
		auto mst = MNSCInitilize(this, 1);
		mst_ = mst;

		CComBSTR script;
		auto bl = MNSCReadUtf8(script_file, &script);

		if (bl)
		{
			_variant_t v1(new IVARIANTApplication(parent), false);

			bl = MNSCParse(mst, script, L"_ap", v1);

			if (bl)
			{
				_variant_t id(0);
				VARIANT v = ScriptCall(mst, L"OnInit", &id, 1);
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

	bool DrowdownListBoxWindowProc(IVARIANTDropdownList* ls, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		
		static CPoint point_prv_(0, 0);
		switch (message)
		{
		case WM_LBUTTONDOWN:
		{
			CPoint point(LOWORD(lParam), HIWORD(lParam));

			int md = ls->SelectRow(point);

			point_prv_ = point;
		}
		break;
		case WM_MOUSEMOVE:
		{
			CPoint point(LOWORD(lParam), HIWORD(lParam));
			int offy = point.y - point_prv_.y;

			if (2 == ls->SelectRow(point) && GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			{
				ls->ScrollbarYoff(offy);
				InvalidateRect(hWnd, NULL, FALSE);
			}

			point_prv_ = point;

		}
		break;
		case WM_LBUTTONUP:
		{
			CPoint point(LOWORD(lParam), HIWORD(lParam));

			int md = ls->SelectRow(point, true);

			if (1 == md || 0 == md)
			{
				ls->ShowDlgListbox(false);
				InvalidateRect(hWnd, NULL, TRUE);
				return UNLOCK_KEEP;
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
				return UNLOCK_KEEP;
			}
		}
		break;
		}

		return LOCK_KEEP;

	}
	bool TextboxWindowProc(IVARIANTTextbox* txt, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		TSF::CTextEditorCtrl* ctrl = (TSF::CTextEditorCtrl*)tsf_.ctrl;

		CRect rc = txt->GetClientRect();
		TSF::TSFApp app(hWnd, rc);

		CPoint pt(LOWORD(lParam), HIWORD(lParam));

		if (!rc.PtInRect(pt) && message == WM_LBUTTONDOWN)
		{
			return UNLOCK_KEEP;
		}

		if (message == WM_BRADCAST_SET_INIT)
		{
			return UNLOCK_KEEP;
		}

		if (1 != ctrl->WndProc(&app, message, wParam, lParam))
			return UNLOCK_KEEP;


		return LOCK_KEEP;

	}

	bool ButtonWindowProc(IVARIANTButton* btn, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{		
		if ( message == WM_LBUTTONUP && btn->enable_ == VARIANT_TRUE)
		{
			CPoint point(LOWORD(lParam), HIWORD(lParam));
			CRect rc = target_->getRect();
			rc.OffsetRect(-2, -2);
			target_->setRect(rc);

			if (target_->getRect().PtInRect(point))
			{
				if (btn->func_onclick_.vt == VT_UNKNOWN) 
				{
					IVARIANTFunction* func = (IVARIANTFunction*)btn->func_onclick_.punkVal;

					VARIANT v1;
					::VariantInit(&v1);
					v1.vt = VT_UNKNOWN;
					v1.punkVal = btn;

					VARIANT v2 = func->Invoke(L"NONAME", &v1, 1);

					::VariantClear(&v2);

				}
				else
				{
					auto btn = dynamic_cast<IVARIANTAbstract*>(target_);

					_variant_t v1(btn);

					VARIANT v = ScriptCall(mst_, L"OnClick", &v1, 1);

					::VariantClear(&v);
				}
			}

			rc.InflateRect(2,2);
			InvalidateRect(hWnd, &rc, TRUE);

			return UNLOCK_KEEP;
		}
		
		return LOCK_KEEP;
	}

	bool ListboxWindowProc(IVARIANTListbox* ls, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
			case WM_LBUTTONUP:
			{	
				CPoint point(LOWORD(lParam), HIWORD(lParam));				
				CRect rc = target_->getRect();

				if ( rc.PtInRect(point))
				{
					auto ls = dynamic_cast<IVARIANTListbox*>(target_);

					CPoint point(LOWORD(lParam), HIWORD(lParam));
					if (1 == ls->SelectRow(point, true))
					{
						//InvalidateRect(hWnd, NULL, FALSE);
					}

					return UNLOCK_KEEP;

				}
			}
			break;
			case WM_MOUSEMOVE:
			{
				auto ls = dynamic_cast<IVARIANTListbox*>(target_);

				CPoint point(LOWORD(lParam), HIWORD(lParam));
				int offy = point.y - point_prv_.y;
				CRect rc = target_->getRect();

				if (GetAsyncKeyState(VK_LBUTTON) & 0x8000 && 2 == ls->SelectRow(point))
				{
					ls->ScrollbarYoff(offy);
					InvalidateRect(hWnd, &rc, FALSE);
				}

				point_prv_ = point;

				return LOCK_KEEP;
				
				
			}
			break;
		}

		return UNLOCK_KEEP;
	}

};