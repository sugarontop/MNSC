#pragma once

#include "TextLayout.h"
#include "IBridgeTSFInterface.h"
#include "TextContainer.h"

namespace TSF {
class CTextStore;
class CTextEditSink;
class CTextContainer;

struct TSFApp
{
	TSFApp(HWND h, const CRect& rc):hWnd(h), rcText_(rc){}

	HWND hWnd;
	CRect rcText_;

};

struct TextInfo
{
	int decoration_start_pos;
	int decoration_end_pos;
	int decoration_typ;
};




class CTextEditor 
{
	public:
		CTextEditor();
		virtual ~CTextEditor(); 
    
		void SetContainer( CTextContainer* ct ){ ct_ = ct; }
		CTextContainer* GetContainer(){ return ct_; }
    
		void MoveSelection(int nSelStart, int nSelEnd, bool bTrail=true);
		BOOL MoveSelectionAtPoint(CPoint pt);
		BOOL InsertAtSelection(LPCWSTR psz);
		BOOL DeleteAtSelection(BOOL fBack);
		BOOL DeleteSelection();

		void MoveSelectionNext();
		void MoveSelectionPrev();
		
		BOOL MoveSelectionUpDown(BOOL bUp, bool bShiftKey, int col,int yoff=1);
		UINT MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey);

		//void Render(D2DContext& cxt, DWRITE_TEXT_METRICS*ptm, ID2D1SolidColorBrush* br, IDWriteTextFormat* tf );
		//void CalcRender(D2DContext& hdc, IDWriteTextFormat* tf );

		void SetFont(CDC& cDC);
		void Draw(CDC& cDC, bool readonly );

		void InnerDraw(CDC& cDC, bool readonly);

		void CalcRender(CDC& cDC, bool readonly);
		
		

		int GetSelectionStart() const {return ct_->SelStart();}
		int GetSelectionEnd() const {return ct_->SelEnd();}
		
		
		HWND GetWnd() const {return hWnd_;}
		TfEditCookie GetCookie() const { return ecTextStore_; }

		void OnComposition( int msg, int len );

		BOOL InitTSF(HWND hWnd, ITfThreadMgr2* tmgr, TfClientId clientid);
		BOOL CloseTSF();

		RECT ClientToScreenMFC(RECT rc) const;
		void SetFocus(D2DMat* pmat);
		void UnsetFocus();
		void Reset( IBridgeTSFInterface* rect_size );

		void InvalidateRect();
		void InvalidateRect2(CRect rc);
		float GetLineHeight() {return layout_.GetLineHeight();}
		CTextLayout *GetLayout() {return &layout_;}
		int CurrentCaretPos();
		void ClearCompositionRenderInfo();
		BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
		UINT GetTextLength() const { return ct_->GetTextLength(); }

		RECT CandidateRect( RECT rc) const;
		int GetRowText( std::wstring* pstr);

		BOOL IsImeOn() const;
		void OnChangeIME(bool bOn);
		BOOL SelectionTab(BOOL bIns);
		void Undo();

		int RowCount() const { return layout_.char_rects_.RowCount(); }
		
		// scrollbar
		bool ScrollByWheel(bool bup);
		
		void OnRefresh(bool bc);
		void CaretUpdate(int md=0);
	protected :
		CRect InitVScollbar(CSize viewsz, int rowcount, int item_height);
		CRect InitHScollbar(CSize viewsz, int rowcount, int item_height);
		void DrawScrollbar(CDC& cDC, int rowcnt);
		
		void ScrollbarYoff(int offy);
		void ScrollbarXoff(int offx, bool bCaretUpdate = true);
		void ScrollbarRowoff(int off_row, bool bCaretUpdate=true);

		void AdjustHScrollbar();
		void SyncScrollbarX(float bai=1.0f);
		
public:
		CTextLayout layout_;
		CTextContainer* ct_;
		IBridgeTSFInterface* bri_;	
		
	protected :
		HWND hWnd_;
		float search_x_;
		CTextEditSink* pTextEditSink_;	
		TextInfo ti_;
		CBitmap bmpText_;
		bool rebuild_;
		
		CSize view_sz_;
	private:
		D2DMat* pmat_;
		ITfThreadMgr2* weak_tmgr_;
		CTextStore* pTextStore_;		
		COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
		int nCompositionRenderInfo_;
		TfEditCookie ecTextStore_;
		ITfDocumentMgr* pDocumentMgr_;
		ITfContext* pInputContext_;
		
		LOGFONT lf_;
		TextInfoEx tix_;
};


class CTextEditSink : public ITfTextEditSink
{
	public:
		CTextEditSink(CTextEditor *pEditor);

		//
		// IUnknown methods
		//
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		//
		// ITfTextEditSink
		//
		STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);
		

		HRESULT _Advise(ITfContext *pic);
		HRESULT _Unadvise();

		std::function<void()> OnChanged_;

	private:
		long _cRef;
		ITfContext *_pic;
		DWORD _dwEditCookie;
		CTextEditor *_pEditor;
};

#ifdef _WINDOWS

class CTextEditorCtrl : public CTextEditor
{
	public :
		HWND Create(HWND hwndParent, ITfThreadMgr2* tmgr, TfClientId clientid);

		LRESULT  WndProc(TSFApp* d, UINT message, WPARAM wParam, LPARAM lParam);
		
		void SetContainer( CTextContainer* ct, IBridgeTSFInterface* ib );
		
		CTextEditor& GetEditor(){ return *this; }

		void OnEditChanged();

		void Password(bool bl){ layout_.Password(bl); }

		void Clear();

		bool CopyBitmap(CBitmap* dstbmp);


		
		
	private :
		
		CPoint MousePoint(CRect rc, LPARAM lParam);
		int ScrollbarMousePoint(CRect rc, LPARAM lParam);
		
		BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
		void OnLButtonDown(float x, float y);
		void OnLButtonUp(float x, float y);
		void OnMouseMove(float x, float y, bool bLbutton);
		void DblClickSelection();

	private:
		int SelDragStart_;
		

};

#endif

};
