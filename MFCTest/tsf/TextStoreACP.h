#pragma once

namespace TSF {

class CTextEditor;


class CTextStore : 
	public ITextStoreACP2, 
	public ITfContextOwnerCompositionSink,
	public ITfCompartmentEventSink,
	public ITfKeyEventSink,
	public ITfKeyTraceEventSink
{
	public:
		 CTextStore(CTextEditor *pEditor);

		//
		// IUnknown methods
		//
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		//
		// ITextStoreACP2
		//
		STDMETHODIMP    AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask);
		STDMETHODIMP    UnadviseSink(IUnknown *punk);

		STDMETHODIMP    RequestLock(DWORD dwLockFlags, HRESULT *phrSession);
		STDMETHODIMP    GetStatus(TS_STATUS *pdcs);
		STDMETHODIMP    QueryInsert(LONG acpInsertStart, LONG acpInsertEnd, ULONG cch, LONG *pacpResultStart, LONG *pacpResultEnd);
		STDMETHODIMP    GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection, ULONG *pcFetched);
		STDMETHODIMP    SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection);
		STDMETHODIMP    GetText(LONG acpStart, LONG acpEnd, __out_ecount(cchPlainReq) WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut, TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut, LONG *pacpNext);
		STDMETHODIMP    SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, __in_ecount(cch) const WCHAR *pchText, ULONG cch, TS_TEXTCHANGE *pChange);
		STDMETHODIMP    RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs);
		STDMETHODIMP    RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags);
		STDMETHODIMP    FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset);
		STDMETHODIMP    RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched);
		STDMETHODIMP    GetEndACP(LONG *pacp);
		STDMETHODIMP    GetActiveView(TsViewCookie *pvcView);
		STDMETHODIMP    GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped);
		STDMETHODIMP    GetScreenExt(TsViewCookie vcView, RECT *prc);
		STDMETHODIMP    GetWnd(TsViewCookie vcView, HWND *phwnd);
		STDMETHODIMP    InsertTextAtSelection(DWORD dwFlags, __in_ecount(cch) const WCHAR *pchText, ULONG cch, LONG *pacpStart,LONG *pacpEnd, TS_TEXTCHANGE *pChange);
		STDMETHODIMP    InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject, LONG *pacpStart,LONG *pacpEnd, TS_TEXTCHANGE *pChange);

		// ITfContextOwnerCompositionSink
		STDMETHODIMP OnStartComposition(ITfCompositionView *pComposition,	BOOL *pfOk	);
		STDMETHODIMP OnEndComposition(ITfCompositionView *pComposition);
		STDMETHODIMP OnUpdateComposition(ITfCompositionView *pComposition,ITfRange  *pRangeNew);

		// ITfCompartmentEventSink
		STDMETHODIMP OnChange(REFGUID rguid);


		// ITfKeyEventSink
		STDMETHODIMP OnSetFocus(/* [in] */ BOOL fForeground){ return S_OK; }
		STDMETHODIMP OnTestKeyDown(	/* [in] */ ITfContext* pic,	/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam,/* [out] */ BOOL* pfEaten){ return S_OK; }
		STDMETHODIMP OnTestKeyUp(	/* [in] */ ITfContext* pic,	/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam,/* [out] */ BOOL* pfEaten){ *pfEaten = TRUE; return S_OK; }
		STDMETHODIMP OnKeyDown(/* [in] */ ITfContext* pic,/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam,/* [out] */ BOOL* pfEaten){ *pfEaten = TRUE; return S_OK; }
		STDMETHODIMP OnKeyUp(/* [in] */ ITfContext* pic,	/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam,/* [out] */ BOOL* pfEaten){ *pfEaten = TRUE; return S_OK; }
		STDMETHODIMP OnPreservedKey(/* [in] */ ITfContext* pic,/* [in] */ REFGUID rguid,	/* [out] */ BOOL* pfEaten){ *pfEaten = TRUE; return S_OK; }

		// ITfKeyTraceEventSink
		STDMETHODIMP OnKeyTraceDown(/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam){ return S_OK; }
		STDMETHODIMP OnKeyTraceUp(/* [in] */ WPARAM wParam,/* [in] */ LPARAM lParam){ return S_OK; }


		// not implement
		STDMETHODIMP    QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable){ return E_NOTIMPL; }
		STDMETHODIMP    GetACPFromPoint(TsViewCookie vcView, const POINT *pt, DWORD dwFlags, LONG *pacp){ return E_NOTIMPL; }
		STDMETHODIMP    RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags){ return E_NOTIMPL; }
		STDMETHODIMP    GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject **ppDataObject){ return E_NOTIMPL; }
		STDMETHODIMP    GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown **ppunk){ return E_NOTIMPL; }
		STDMETHODIMP    InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject *pDataObject, TS_TEXTCHANGE *pChange){ return E_NOTIMPL; }


	public :
		void OnSelectionChange();
		void OnTextChange(LONG acpStart, LONG acpOldEnd, LONG acpNewEnd);
		void OnLayoutChange();
   
		void InitSink(ITfThreadMgr2* mgr, TfClientId TfClientId);
		void CloseSink(ITfThreadMgr2* mgr);


		std::function<void()> selection_changed;

	private:
		void PrepareAttributes(ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs);
   
		CTextEditor *_pEditor;	
		ITextStoreACPSink* TextStoreACPSink_;
		TS_ATTRVAL _attrval[8];
		int _nAttrVals;
		long _cRef;
		DWORD opencloseck_;
		CComPtr<ITfThreadMgr2> mgr2_;

		

		// from tsfapp.
		// lock,lock,lock
		BOOL	m_fLocked;
	    DWORD	m_dwLockType;
		BOOL	m_fPendingLockUpgrade;
		BOOL _LockDocument(DWORD dwLockFlags);
		void _UnlockDocument();
		BOOL _IsLocked(DWORD dwLockType);
};


};
