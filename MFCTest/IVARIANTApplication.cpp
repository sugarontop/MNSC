#include "pch.h"

#include "MFCTest.h"
#include "MFCTestDoc.h"
#include "MFCTestView.h"
#include <msxml6.h>
#include "mnsc.h"
#include "tsf\D2DMisc.h"
#include "x.h"
#include "x2.h"
#include "x3.h"
#include "x4.h"
#include "x5.h"
#include "x6.h"
#include "IVARIANTApplication.h"
#include "MessageLayerPlate.h"


#pragma comment( lib, "msxml6")

VARIANT IVARIANTApplication::create_object(VARIANT vid, VARIANT typ, VARIANT v)
{
	if (v.vt == VT_UNKNOWN && typ.vt == VT_BSTR && vid.vt == VT_INT)
	{
		int layer_idx = vid.intVal;
		CComBSTR objtyp = typ.bstrVal;
		IVARIANTMap* par = dynamic_cast<IVARIANTMap*>(v.punkVal);
		if (par)
		{
			_variant_t x, y, cx, cy, text, brd, readonly, bmultiline,fontheight;
			if (!par->GetItem(L"x", &x))
				x = 0;
			if (!par->GetItem(L"y", &y))
				y = 0;
			if (!par->GetItem(L"cx", &cx))
				cx = 200;
			if (!par->GetItem(L"cy", &cy))
				cy = 30;
			if (!par->GetItem(L"text", &text))
				text = L"";
			if (!par->GetItem(L"border", &brd))
				brd = FALSE;
			if (!par->GetItem(L"multiline", &bmultiline))
				bmultiline = FALSE;
			

			if (objtyp == L"button")
			{
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);
				auto p = new IVARIANTButton(rc);
				auto obj = dynamic_cast<DrawingObject*>(p);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));

				VARIANT vfunc;
				::VariantInit(&vfunc);

				if (par->GetItem(L"onclick", &vfunc))
					p->func_onclick_.Attach(vfunc);



				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = dynamic_cast<IVARIANTButton*>(obj);
				v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"listbox")
			{
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
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto txt = new IVARIANTTextbox(rc, bmultiline.boolVal);

				pview_->uilayers_[layer_idx]->objects_.push_back(txt);

				txt->setText(std::wstring(text.bstrVal));

				txt->setProperty(par);

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = txt;
				v1.punkVal->AddRef();
				return v1;
			}
			else if (objtyp == L"dropdownlist")
			{
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
				// ƒeƒXƒg
				CRect rc(x.intVal, y.intVal, x.intVal + cx.intVal, y.intVal + cy.intVal);

				auto cav = new IVARIANTCanvas(rc);
				auto obj = dynamic_cast<DrawingObject*>(cav);
				pview_->uilayers_[layer_idx]->objects_.push_back(obj);

				obj->setText(std::wstring(text.bstrVal));

				VARIANT v1;
				::VariantInit(&v1);
				v1.vt = VT_UNKNOWN;
				v1.punkVal = cav;
				v1.punkVal->AddRef();
				return v1;
			}


		}
	}

	_variant_t ret(0);
	return ret;
}