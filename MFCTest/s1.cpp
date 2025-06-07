#include "pch.h"
#include "x3.h"


#include "tsf\TextEditor.h"
#include "tsf\TextStoreACP.h"


static void THR(HRESULT x) { if (x != S_OK) throw x; }

TSFIsland tsf_;

bool TSFInit(HWND hWnd)
{
	try
	{
		THR(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&tsf_.pThreadMgr));
		THR(tsf_.pThreadMgr->Activate(&tsf_.TfClientId));
		THR(tsf_.pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&tsf_.pKeystrokeMgr));
		THR(CoCreateInstance(CLSID_TF_DisplayAttributeMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfDisplayAttributeMgr, (void**)&tsf_.DisplayAttributeMgr));

		auto ctrl = new TSF::CTextEditorCtrl();
		ctrl->Create(hWnd, tsf_.pThreadMgr, tsf_.TfClientId);

		tsf_.ctrl = ctrl;
		tsf_.hWnd = hWnd;
				

		return true;
	}
	catch (HRESULT)
	{
		

		return false;
	}
}
void TSFClose(HWND hWnd)
{
	auto p = ((TSF::CTextEditorCtrl*)tsf_.ctrl);
	p->CloseTSF();
	delete p;

}
TSF::CTextEditorCtrl* IVARIANTTextbox::ctrl() const
{
	return (TSF::CTextEditorCtrl*)tsf_.ctrl;
}