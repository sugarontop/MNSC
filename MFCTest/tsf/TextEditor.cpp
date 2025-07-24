#include "pch.h"
#include "TextEditor.h"
#include "TextStoreACP.h"
#include <Imm.h>
using namespace TSF;



#define TES_INVALID_COOKIE  ((DWORD)(-1))
#define WM_D2D_ONIME_ONOFF (WM_APP+20)
#define SCROLLBAR_WIDTH	    14
#define VSCROLL_POINT_MODE 1
#define HSCROLL_POINT_MODE 2


CTextEditor::CTextEditor() 
{
    pTextStore_ = NULL;    
	ct_ = NULL;
	pCompositionRenderInfo_ = NULL;
    nCompositionRenderInfo_ = 0;
	pDocumentMgr_ = NULL;
	pInputContext_ = NULL;
    search_x_ = 0;
    weak_tmgr_ = NULL;
    rebuild_ = true;
    lf_ = {};
}

CTextEditor::~CTextEditor() 
{
    CloseTSF();
	
}

//----------------------------------------------------------------
//
// Application Initialize
//
//----------------------------------------------------------------

BOOL CTextEditor::InitTSF(HWND hWnd, ITfThreadMgr2* tmgr, TfClientId clientid)
{
	BOOL ret = FALSE;

    pTextStore_ = new CTextStore(this);

    weak_tmgr_ = tmgr;

	ITfDocumentMgr* pDocumentMgrPrev = NULL;
    CComPtr<IUnknown> q;

    if (!pTextStore_) 
		goto Exit;
    
	if (FAILED(tmgr->CreateDocumentMgr(&pDocumentMgr_)))
		goto Exit;


   
    pTextStore_->QueryInterface( IID_IUnknown, (void**)&q);

    if (FAILED(pDocumentMgr_->CreateContext(clientid, 0, q, &pInputContext_, &ecTextStore_)))
		goto Exit;

    if (FAILED(pDocumentMgr_->Push(pInputContext_)))
		goto Exit;

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
    tmgr->SetFocus(pDocumentMgr_);
#else
	if (FAILED( g_pThreadMgr->AssociateFocus(hWnd, pDocumentMgr_, &pDocumentMgrPrev)))
		goto Exit;
#endif
	
    pTextStore_->InitSink(tmgr, clientid);
    


	hWnd_ = hWnd;

	if ( pDocumentMgrPrev )
		pDocumentMgrPrev->Release();

	pTextEditSink_ = new CTextEditSink(this);
    if (!pTextEditSink_)
		goto Exit;

    pTextEditSink_->_Advise(pInputContext_);


    pTextStore_->selection_changed = std::bind(&CTextEditorCtrl::OnRefresh, this, true);

	ret = TRUE;

Exit :
	if ( pDocumentMgrPrev )
		pDocumentMgrPrev->Release();
    
    return ret;
}
  

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::CloseTSF( )
{
    if (pTextEditSink_)
    {
        pTextEditSink_->_Unadvise();
        pTextEditSink_->Release();
        pTextEditSink_ = NULL;
    }
	
	if (pDocumentMgr_)
    {
        pDocumentMgr_->Pop(TF_POPF_ALL);
		
		pDocumentMgr_->Release();
		pDocumentMgr_ = NULL;
	}

	if (pInputContext_)
	{
		pInputContext_->Release();
		pInputContext_ = NULL;
	}

    if (pTextStore_)
    {
        pTextStore_->CloseSink(weak_tmgr_);


        while( pTextStore_->Release() );
        pTextStore_ = NULL;
    }

    return TRUE;
}
//----------------------------------------------------------------
//
// move Caret 
//
//----------------------------------------------------------------

void CTextEditor::MoveSelection(int nSelStart, int nSelEnd, bool bTrail)
{
	if ( nSelEnd < nSelStart )
		std::swap( nSelStart, nSelEnd );
	
	if ( ct_ )
	{
		int nTextLength = (int)ct_->GetTextLength();
		if (nSelStart >= nTextLength)
			nSelStart = nTextLength;

		if (nSelEnd >= nTextLength)
			nSelEnd = nTextLength;

		// set caret position

		ct_->SetSelStart(nSelStart); 
		ct_->SetSelEnd(nSelEnd);

		ct_->bSelTrail_ = bTrail;

    

		pTextStore_->OnSelectionChange();
	}
}


//----------------------------------------------------------------
//
//	caretが動く時
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionNext()
{
    int nTextLength = (int)ct_->GetTextLength();

	int zCaretPos = (ct_->bSelTrail_ ? ct_->SelEnd() : ct_->SelStart() );
	zCaretPos = min(nTextLength, zCaretPos+1); // 1:次の文字

    ct_->SetSelStart(zCaretPos);
    ct_->SetSelEnd(zCaretPos);
    pTextStore_->OnSelectionChange();
}

//----------------------------------------------------------------
//
//　caretが動く時
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionPrev()
{
	int zCaretPos = (ct_->bSelTrail_ ? ct_->SelEnd() : ct_->SelStart() );
	zCaretPos = max(0, zCaretPos-1);


    ct_->SetSelStart(zCaretPos);
    ct_->SetSelEnd(zCaretPos);

    pTextStore_->OnSelectionChange();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
BOOL CTextEditor::MoveSelectionAtPoint(CPoint ptlog)
{
    BOOL bRet = FALSE;
    int nSel = (int)layout_.CharPosFromPoint(ptlog);
    if (nSel != -1)
    {
        MoveSelection(nSel, nSel, true);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
// VK_DOWN,VK_UP
//
//----------------------------------------------------------------
BOOL CTextEditor::MoveSelectionUpDown(BOOL bUp, bool bShiftKey, int col, int yoff )
{
    int caret = CurrentCaretPos();

    UINT nSel = bUp ? ct_->SelEnd() : ct_->SelStart() ;

    auto rc = layout_.RowCol(caret);
    rc.col = col;
    int zpos = 0;
    if ( bUp )
    {
        rc.row = max(0, rc.row - yoff);
        zpos = layout_.ZPos(rc);        
    }
    else
    {
        rc.row = max(0, rc.row + yoff);
        zpos = layout_.ZPos(rc);
    }

    int& top_row_idx = ct_->top_row_idx_;

    
    // scrollbarのtrim
    if ( rc.row - top_row_idx < 0 )
    {
        top_row_idx = max(0, top_row_idx - yoff);
        
        rc.row = top_row_idx;
        ScrollbarRowoff(0); // set scrollbar
        zpos = layout_.ZPos(rc);
    }
    else 
    {
        while ( (view_sz_.cy- GetLineHeight()) < GetLineHeight()*(rc.row- top_row_idx))
            top_row_idx++;

        ScrollbarRowoff(0);
        zpos = layout_.ZPos(rc);
    }


    if (bShiftKey)
        MoveSelection(nSel, zpos, !bUp);
    else
        MoveSelection(zpos, zpos, !bUp);

    return TRUE;


    return FALSE;

}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextEditor::MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey)
{
    BOOL bRet = FALSE;
    UINT nSel,nSel2;

    if (bFirst)
    {
		// when pushed VK_HOME
		ct_->nStartCharPos_ = 0;
		nSel2 = ct_->SelEnd();
        nSel = layout_.FineFirstEndCharPosInLine(ct_->SelStart(), TRUE);
    }
    else
    {
        // when pushed VK_END
		ct_->nStartCharPos_ = 0;
		nSel2 = ct_->SelStart();
		nSel = layout_.FineFirstEndCharPosInLine(ct_->SelEnd(), FALSE);
    }

    if (nSel != (UINT)-1)
    {
		if ( bShiftKey )
		{
			MoveSelection(nSel, nSel2,true);
		}
		else
		{
			MoveSelection(nSel, nSel);
		}

        bRet = TRUE;
    }

	
    return nSel;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::InvalidateRect()
{
#ifdef _WINDOWS
    ::InvalidateRect(hWnd_, NULL, FALSE); //FALSE);
#endif
}
void CTextEditor::InvalidateRect2(CRect rc)
{
#ifdef _WINDOWS
    ::InvalidateRect(hWnd_, &rc, FALSE); //FALSE);
#endif
}

//----------------------------------------------------------------
//
// カレント行とtab数を取得
//
//----------------------------------------------------------------
int CTextEditor::GetRowText(std::wstring* str)
{
	auto s = ct_->GetRowText(ct_->SelStart());
	
	int cnt = 0;

	for(int i=0; i < s.length(); i++)
	{
		if ( s[i] == '\t')
			cnt++;
		else
			break;
	}

	if ( str )
		*str = s;

	return cnt;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
    //layout_.bRecalc_ = true;
   
	LONG lOldSelEnd = ct_->SelEnd();
	if (!ct_->RemoveText(ct_->SelStart(), ct_->SelEnd() - ct_->SelStart()))
		return FALSE;

	UINT nrCnt;
    if (!ct_->InsertText(ct_->SelStart(), psz, (UINT)wcslen(psz), nrCnt))
        return FALSE;
	
    ct_->SetSelStart( ct_->SelStart()+ (int)wcslen(psz));
    ct_->SetSelEnd( ct_->SelStart());
    
	LONG acs = ct_->SelStart();
	LONG ecs = ct_->SelEnd();

	pTextStore_->OnTextChange(acs, lOldSelEnd, ecs);
    pTextStore_->OnSelectionChange();
    return TRUE;
}
//----------------------------------------------------------------
//
// Selecttion行の先頭にTAB追加　先頭のTAB削除
//
//----------------------------------------------------------------
BOOL CTextEditor::SelectionTab(BOOL bDel)
{
	LONG acs = ct_->SelStart();
	LONG ecs = ct_->SelEnd();
	LONG ecs2 = ecs;

	int ns = ct_->LineNo(ct_->SelStart());
	int ne = ct_->LineNo(ct_->SelEnd());

	int cnt = ne-ns;

	if ( cnt == 0 )
		return FALSE;

	for(int i = 0; i <= cnt; i++)
		ecs2 = ct_->AddTab(i+ns, !bDel);
	
	ct_->SetSelStart(acs);
	ct_->SetSelEnd(ecs2);

	pTextStore_->OnTextChange(acs, ecs, ecs2);
    pTextStore_->OnSelectionChange();

	return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
    //layout_.bRecalc_ = true;

    if (!fBack && (ct_->SelEnd() < (int)ct_->GetTextLength()))
    {
        if (!ct_->RemoveText(ct_->SelEnd(), 1))
            return FALSE;
		
		LONG ecs = ct_->SelEnd();

        pTextStore_->OnTextChange(ecs, ecs + 1, ecs);
    }
	 
    if (fBack && (ct_->SelStart() > 0))
    {
        if (!ct_->RemoveText(ct_->SelStart() - 1, 1))
            return FALSE;

        ct_->SetSelStart( ct_->SelStart() -1);
        ct_->SetSelEnd( ct_->SelStart());

		LONG acs = ct_->SelStart();
        pTextStore_->OnTextChange(acs, acs + 1, acs );
        pTextStore_->OnSelectionChange();
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteSelection()
{

    ULONG nSelOldEnd = ct_->SelEnd();
    ct_->RemoveText(ct_->SelStart(), ct_->SelEnd() - ct_->SelStart());

    ct_->SetSelEnd( ct_->SelStart());


	LONG acs = ct_->SelStart();

    pTextStore_->OnTextChange(acs, nSelOldEnd, acs);
    pTextStore_->OnSelectionChange();

    return TRUE;
}
 
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
int CTextEditor::CurrentCaretPos()
{
    if ( ct_ )
	    return (ct_->bSelTrail_ ? ct_->SelEnd() : ct_->SelStart() );
    return 0;
}




//----------------------------------------------------------------
//
// aHahaha, ActionCaret
//
//----------------------------------------------------------------
static int g_caret = 0;

void ActionCareteCaret(HWND hWnd, int height)
{
    ::CreateCaret(hWnd, 0, 4, (int)height); 
    g_caret = 0;
}
void ActionCaret(HWND hWnd, BOOL show)
{
    if ( show )
    {
        while(g_caret < 1)
        {
            ShowCaret(hWnd);
            g_caret++;
        }
    }
    else 
    {
        HideCaret(hWnd);
        g_caret--;        
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::SetFont(CDC& cDC)
{
    cDC.GetCurrentFont()->GetLogFont(&lf_);
}
void CTextEditor::Draw(CDC& cDC, bool readonly)
{
    if (ct_->vscbar_rc_.bottom == 0 ) 
    {
        int rowcount = RowCount();
        int item_height = GetLineHeight();

        view_sz_ = ct_->rc_.Size();
        ct_->vscbar_rc_ = InitVScollbar(view_sz_, rowcount, item_height);
        ct_->hscbar_rc_ = InitHScollbar(view_sz_, rowcount, item_height);

    }

    int selstart = (int)ct_->SelStart(); 
    int selend = (int)ct_->SelEnd();
 

    bool b1 = ct_->bSelTrail_;
    int pos = CurrentCaretPos();

    int cx = ct_->rc_.Width();
    int cy = ct_->rc_.Height();
    int cy2 = ct_->rc_.Height() - (ct_->IsShowHScrollbar() ? SCROLLBAR_WIDTH : 0);

    bmpText_.DeleteObject();

    CDC memDC;
    memDC.CreateCompatibleDC(&cDC);
    bmpText_.CreateCompatibleBitmap(&cDC, cx, cy);
    auto oldb = memDC.SelectObject(&bmpText_);
    CRect rc(0, 0, cx, cy);
    memDC.FillSolidRect(rc, RGB(255, 255, 255));

    auto oldf = memDC.SelectObject(cDC.GetCurrentFont());

    // memDCに文字を表示
    CPoint view_org = layout_.Draw(memDC, ct_->top_row_idx_,cx,cy2, ct_->GetTextBuffer(), (int)ct_->GetTextLength(), selstart, selend, ct_->bSelTrail_, pos, ct_->scrollbar_offx_);

    BitBlt(cDC, ct_->rc_.left, ct_->rc_.top, cx, cy, memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldb);
    memDC.SelectObject(oldf);


    // 選択範囲の表示 
    cDC.SaveDC();
    cDC.SetViewportOrg(view_org);
    layout_.DrawSelectRange(cDC, ct_->rc_, selstart, selend);
    cDC.RestoreDC(-1);

    // Draw Scrollbar
    if ( 1 < layout_.RowCount() )
    {
        cDC.SaveDC();
        cDC.OffsetViewportOrg(ct_->rc_.left, ct_->rc_.top);
        DrawScrollbar(cDC);
        cDC.RestoreDC(-1);
    }
}

void CTextEditor::OnRefresh(bool bc)
{    
    // get char rects.
    if ( lf_.lfHeight != 0)
    {
        CClientDC  cDC(nullptr);
        CFont cf;
        cf.CreateFontIndirect(&lf_);
        auto oldf = cDC.SelectObject(&cf);
        CalcRender(cDC, false);
        cDC.SelectObject(&oldf);
    }

    // show caret and adjust scrollbar.
    if (bc)
    {
        auto offpt = layout_.Offset();
        int pos = CurrentCaretPos();

        // caretの表示
        CRect caretRect = { 0,0,0,0 };
        if (!layout_.char_rects_.empty())
        {
            //cDC.OffsetViewportOrg(ct_->rc_.left, ct_->rc_.top);

            CRect rcStart;
            bool blf1 = false;
            layout_.RectFromCharPosEx(pos - 1, -1, &rcStart, &blf1);

            if (rcStart.top - ct_->top_row_idx_ * layout_.GetLineHeight() < 0)
            {
                CPoint pt(rcStart.left, 0);

                int pos = layout_.CharPosFromPoint(pt);

                //bool ct_->SelTrail() const { return bSelTrail_; }
                ct_->SetSelStart(pos);
                ct_->SetSelEnd(pos);

                layout_.RectFromCharPos((int)pos, &rcStart);
            }

            caretRect = rcStart;

            caretRect.left = caretRect.right;
            caretRect.right = caretRect.right + 4;

            //cDC.OffsetViewportOrg(-ct_->rc_.left, -ct_->rc_.top);
        }

        caretRect.OffsetRect(-offpt.x, -offpt.y);

        int x = ct_->rc_.left + caretRect.left;
        int y = ct_->rc_.top + caretRect.top;

        bool bShowCaret = (0 <= caretRect.top && caretRect.bottom < view_sz_.cy
            && 0 <= caretRect.left && caretRect.right < view_sz_.cx);

        ActionCaret(hWnd_, FALSE);

        int cy = ct_->rc_.Size().cy;
        int vsb_h = SCROLLBAR_WIDTH;
        int cy2 = caretRect.bottom;

        if (cy2 > cy - vsb_h)
        {            
            ScrollbarRowoff(1);
            y += -caretRect.Height();

            bShowCaret = true;
        }

        ::SetCaretPos(x, y);

        if (bShowCaret)
            ActionCaret(hWnd_, TRUE);
    }
}
void CTextEditor::DrawScrollbar(CDC& cDC)
{
    CBrush brb,br;

    brb.CreateSolidBrush(RGB(220,220,220));
    br.CreateSolidBrush(RGB(20, 20, 20));
   
    if ( ct_->IsShowVScrollbar())
    {
        CRect v1(ct_->vscbar_rc_);
        v1.top = 0;
        v1.bottom = ct_->rc_.Height();
        cDC.FillRect(v1, &brb);
        cDC.FillRect(ct_->vscbar_rc_, &br);
    }

    if (ct_->IsShowHScrollbar())
    {
        CRect v1(ct_->hscbar_rc_);
        v1.left = 0;
        v1.right = ct_->rc_.Width();
        cDC.FillRect(v1, &brb);
        cDC.FillRect(ct_->hscbar_rc_, &br);
    }
}


static bool s_create_caret_;

void CTextEditor::CalcRender(CDC& cDC, bool readonly)
{
	if (layout_.Recalc() == false ) return;
    
    int h = (int)layout_.GetLineHeight();

    int zCaretPos = CurrentCaretPos();

    if (rebuild_)
    {
        layout_.CreateLayout(cDC, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_, zCaretPos, ct_->nStartCharPos_);
        rebuild_ = false;
    }
    else
        layout_.ReCreateLayout(cDC, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_, zCaretPos, ct_->nStartCharPos_);

    layout_.SetRecalc(false);

    ct_->line_width_max_ = layout_.GetLineWidthMax();
    ct_->line_height_max_ = GetLineHeight()*RowCount();

    
    if (s_create_caret_ && readonly == false)
    {
        h = layout_.GetLineHeight();
        

        ActionCareteCaret(hWnd_,h);

        ActionCaret(hWnd_, FALSE);

        s_create_caret_ = false;
    }
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------



RECT CTextEditor::CandidateRect(RECT rclog) const
{    
     if ( pmat_)
     {
        D2DMat map = *pmat_;

        map._31 += -ct_->scrollbar_offx_;
        map._32 += -(int)(layout_.GetLineHeight() * ct_->top_row_idx_);

	    FRectF xrc = map.LPtoDP(rclog);
        
	    RECT ret = this->ClientToScreenMFC( xrc.GetRECT());

	    return ret;     
    }

	RECT ret = {};
    return ret;
}



BOOL CTextEditor::IsImeOn() const
{
	CComPtr<ITfCompartmentMgr>  pCompartmentMgr;
	BOOL isIMEOn = FALSE;

	if (0 == weak_tmgr_->QueryInterface(IID_ITfCompartmentMgr, (LPVOID*)&pCompartmentMgr))
	{
        CComPtr<ITfCompartment> cp1;

		if (S_OK == pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &cp1))
		{
			VARIANT v;
			cp1->GetValue(&v);

			isIMEOn = !(v.lVal == 0);

		}
	}
	return isIMEOn;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------



void CTextEditor::SetFocus(D2DMat* pmat)
{
    s_create_caret_ = false;
    if (pDocumentMgr_)
    {
        if ( pmat )
			pmat_ = pmat;

        weak_tmgr_->SetFocus(pDocumentMgr_);

        rebuild_ = true;

        s_create_caret_ = true;

        OnRefresh(false);
        //ActionCaret(hWnd_, FALSE);
        
    }
}


//----------------------------------------------------------------
//
// 文字エリアの実設定
//
//----------------------------------------------------------------
void CTextEditor::Reset( IBridgeTSFInterface* bi )
{ 	
	bri_ = bi;
	
	if ( bri_ )
	{
        CRect rc =  bi->GetClientRect();

		ct_->rc_ = rc;		
        ct_->view_size_.cx = (LONG)rc.Width();
		ct_->view_size_.cy =(LONG)(rc.bottom - rc.top);

        if ( bi->GetType() == IBridgeTSFInterface::MULTILINE )
        {
            ct_->view_size_.cx = (LONG)(rc.right - rc.left);
            ct_->hscbar_rc_ = {};
            ct_->vscbar_rc_ = {};
            ct_->top_row_idx_=0;
            ct_->scrollbar_offx_=0;
            ct_->line_width_max_=0;
            ct_->line_height_max_ = 0;

        }
	}
	else
	{
		int a = 0;

	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::ClearCompositionRenderInfo()
{
    if (pCompositionRenderInfo_)
    {
        LocalFree(pCompositionRenderInfo_);
        pCompositionRenderInfo_ = NULL;
        nCompositionRenderInfo_ = 0;
    }
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::OnComposition( int msg, int len )
{
	TRACE( L"void CTextEditor::OnComposition( %d, %d )\n", msg,len);

    switch( msg ) 
    {
        case 1:
        {
            ti_.decoration_typ = 1;

            ti_.decoration_start_pos = GetSelectionStart();
            ti_.decoration_end_pos = GetSelectionEnd();

			
        }
        break;
        case 2:
        {            
            ti_.decoration_start_pos = max(0, GetSelectionEnd() - len);
            ti_.decoration_end_pos =GetSelectionEnd();
			
        }
        break;
        case 3:
        {
            ti_.decoration_typ = 0;
            ti_.decoration_end_pos = 0;
            ti_.decoration_start_pos = 0;
			
			
			
			ct_->UndoAdjust();
        }
        break;
    }
}

void CTextEditor::OnChangeIME(bool bOn)
{
	

	SendMessage(GetWnd(),WM_D2D_ONIME_ONOFF, (bOn?1:0), 0);

}


RECT CTextEditor::ClientToScreenMFC(RECT rc) const
{
    HWND hWnd = GetWnd();

    //auto logicdpi = GetDpiFromDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE); // 150% is 144.
    auto logicdpi = 96.0f;

    ::ClientToScreen(hWnd, (POINT*)&rc.left);
    ::ClientToScreen(hWnd, (POINT*)&rc.right);

    //DIP value = (physical pixel x 96) / DPI
    //physical pixel value = (DIP x DPI) / 96

//    auto bnd = CoreWindow::GetForCurrentThread().Bounds(); // DIP
//    auto view = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

    auto f = [logicdpi](LONG dip)->LONG
    {
        return static_cast<LONG>(dip * logicdpi / 96.0f);
    };

    rc.left = f(rc.left);
    rc.top = f(rc.top);
    rc.right = f(rc.right);
    rc.bottom = f(rc.bottom);

    return rc; // physical pixel
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
    if (pCompositionRenderInfo_)
    {
        void *pvNew = LocalReAlloc(pCompositionRenderInfo_, 
                                   (nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO),
                                   LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)pvNew;
    }
    else
    {
        pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
                                   (nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO));
        if (!pCompositionRenderInfo_)
            return FALSE;
    }
    pCompositionRenderInfo_[nCompositionRenderInfo_].nStart = nStart;
    pCompositionRenderInfo_[nCompositionRenderInfo_].nEnd = nEnd;
    pCompositionRenderInfo_[nCompositionRenderInfo_].da = *pda;
    nCompositionRenderInfo_++;

    return TRUE;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef _WINDOWS

void CTextEditorCtrl::SetContainer( CTextContainer* ct, IBridgeTSFInterface* brt )
{
	CTextEditor::SetContainer(ct);

	Reset(brt);

    if ( ct )
    {
        ct->SetSelStart(0);
        ct->SetSelEnd(0);
    }

    

}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

HWND CTextEditorCtrl::Create(HWND hwndParent, ITfThreadMgr2* tmgr, TfClientId clientid)
{
    hWnd_ = hwndParent;
	
	InitTSF(hWnd_, tmgr, clientid);

	pTextEditSink_->OnChanged_ = std::bind(&CTextEditorCtrl::OnEditChanged, this );

	static CTextContainer dumy;
	SetContainer(  &dumy , NULL );

    
    return hWnd_;
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditorCtrl::OnEditChanged()
{
	// from CTextEditSink::OnEndEdit

    layout_.SetRecalc(true);
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditorCtrl::Clear()
{
    layout_.SetRecalc(true);
    search_x_ = 0;
    ti_ = {};
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
CPoint CTextEditorCtrl::MousePoint(CRect rc, LPARAM lParam)
{
    CPoint pt(GET_X_LPARAM(lParam) - rc.left, GET_Y_LPARAM(lParam) - rc.top);

    pt.x += layout_.Offset().x;
    pt.y += layout_.Offset().y;

    return pt;

}
int CTextEditorCtrl::ScrollbarMousePoint(CRect rc, LPARAM lParam)
{
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    if ( ct_->IsShowVScrollbar())
    {
        CRect rc2(rc);
        rc2.left = rc2.right - SCROLLBAR_WIDTH;
        if ( rc2.PtInRect(pt))
            return VSCROLL_POINT_MODE;
    }

    if ( ct_->IsShowHScrollbar())
    {
        CRect rc3(rc);
        rc3.top = rc3.bottom- SCROLLBAR_WIDTH;
        if ( rc3.PtInRect(pt))
            return HSCROLL_POINT_MODE;
    }    
    return 0;
}


LRESULT CTextEditorCtrl::WndProc(TSFApp* d, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 1;

    auto rc = d->rcText_;
    static int mouse_md = 0;
    static CPoint mpt;

    if ( ct_ )
    {
        switch (message)
        {
            case WM_KEYDOWN:
                if ( this->OnKeyDown(wParam, lParam) )
				    ret = 1;

                if ( (0xff & wParam) == VK_ESCAPE)
                    ret = 0;

                InvalidateRect();
		    break;
            case WM_LBUTTONDOWN:
            {
                search_x_ = 0;
                
                //MouseParam* pm = (MouseParam*)lParam;
                //MFCMatrix m(pm->mat);
                //FPointF pt = m.DPtoLP(pm->pt);

                mouse_md = ScrollbarMousePoint(rc, lParam);

                if (VSCROLL_POINT_MODE == mouse_md)
                {
                    mouse_md = VSCROLL_POINT_MODE;

                    mpt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    ret = 1;

                }
                else if (HSCROLL_POINT_MODE == mouse_md)
                {
                    mouse_md = HSCROLL_POINT_MODE;

                    mpt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    ret = 1;

                }
                else if (mouse_md == 0)
                {
                    CPoint pt = MousePoint(rc, lParam); //(GET_X_LPARAM(lParam)-rc.left, GET_Y_LPARAM(lParam)-rc.top);
                    this->OnLButtonDown( pt.x, pt.y );
                    ret = 1;
				    InvalidateRect();
                }
            }
		    break;
            case WM_MOUSEMOVE:
            {
                /*MouseParam* pm = (MouseParam*)lParam;

                MFCMatrix m(pm->mat);
                FPointF pt = m.DPtoLP(pm->pt);*/

                

                if (VSCROLL_POINT_MODE == mouse_md)
                {
                    CPoint pt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));                    
                    int offy = pt.y - mpt.y;

                    ScrollbarYoff(offy);

                    InvalidateRect();

                    mpt = pt;
                    ret = 1;
                }
                else if (HSCROLL_POINT_MODE == mouse_md)
                {
                    CPoint pt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    int offx = pt.x - mpt.x;

                    ScrollbarXoff(offx);

                    InvalidateRect();

                    mpt = pt;
                    ret = 1;
                }
                else if (mouse_md==0)
                {
                    auto prvscbar_rc = ct_->vscbar_rc_;

                    CPoint pt = MousePoint(rc, lParam);
                    bool bl = wParam & MK_LBUTTON;

                    if ( bl )
                    {
                        CPoint pt2(GET_X_LPARAM(lParam) - rc.left, GET_Y_LPARAM(lParam) - rc.top);
                        
                        if (0 < view_sz_.cy ) {
                        if ( view_sz_.cy < pt2.y )
                            ScrollbarRowoff(1);
                        else if ( pt2.y <0 )
                            ScrollbarRowoff(-1);
                       }
                    }
                    
                    this->OnMouseMove(pt.x, pt.y, bl);

                    
                    if ( prvscbar_rc != ct_->vscbar_rc_ )
                        InvalidateRect();
                    else
                    {
                        CRect xrc(rc);
                        xrc.right -= ct_->vscbar_rc_.Width();
                        xrc.bottom -= ct_->hscbar_rc_.Height();

                        if (bl)
                            InvalidateRect2(xrc);
                    }
                    

                    ret = 1;
                }
            }
            break;
            case WM_LBUTTONUP:
            {
                /*MouseParam* pm = (MouseParam*)lParam;

                MFCMatrix m(pm->mat);
                FPointF pt = m.DPtoLP(pm->pt);*/

                if (VSCROLL_POINT_MODE == mouse_md || HSCROLL_POINT_MODE == mouse_md)
                {
                    mouse_md = 0;
                    ret = 1;
                }
                else if (mouse_md == 0)                
                {
                    CPoint pt = MousePoint(rc, lParam); // (GET_X_LPARAM(lParam) - rc.left, GET_Y_LPARAM(lParam) - rc.top);
                    this->OnLButtonUp(pt.x, pt.y);
                    InvalidateRect();


                    ret = 1;
                }
            }
		    break;
            
            case WM_CHAR:
			{
                // wParam is a character of the result string. 
			    bool bControlKey = ((GetKeyState(VK_CONTROL) & 0x80) != 0);			  
			    if ( bControlKey )
				    return 0;

				bool bShiftKey = ((GetKeyState(VK_SHIFT) & 0x80) != 0);			  

                WCHAR wch = (WCHAR)wParam;
			    // ansi charcter input.
                bool bMultiline = (bri_->GetType() == IBridgeTSFInterface::MULTILINE);
                if ( bMultiline )
                {
					if ( ct_->SelStart() != ct_->SelEnd() && wch == L'\t' )
					{
						// bShiftKey:false 複数行にadd TAB, bShiftKey:true 複数行にdel TAB
						if ( SelectionTab(bShiftKey) )
							return 1;
					}

                    if ( wch >= L' ' ||  (wch == L'\r'&& !ct_->bSingleLine_ ) || wch == L'\t' ) 
                    {				
                        if ( wch < 256 )
                        {
                            if ( wch == L'\r' )
                                wch = L'\n';

							if ( wch != L'\n' )
							{
								const WCHAR wc[2] = { wch, 0 }; 							
								this->InsertAtSelection(wc);
							}
							else
							{
								// \n -> auto insert tab

								WCHAR cb[256]={0};

								int cnt = this->GetRowText(nullptr);

								cb[0] = wch;
								for(int i=0; i < cnt; i++ )
									cb[i+1] = L'\t';
																
								this->InsertAtSelection(cb);
							}
                            ret = 1;
                        }				   
                    }
                }
                else
                {
			        if ( wch >= L' ' ||  wch == L'\t' ) 
                    {				
				        if ( wch < 256 )
				        {
					        const WCHAR wc[2] = { wch, 0 }; 
		                    this->InsertAtSelection(wc);
                            ret = 1;
				        }				   
                    }
                }
                layout_.SetRecalc(true);
			}
		    break;
            case WM_IME_COMPOSITION:
            {
                if (lParam & GCS_RESULTSTR)
                {
                    //needsScrollAdjust = true; // 確定時にフラグをセット
                    //InvalidateRect(hwndEdit, NULL, TRUE); // WM_PAINTをトリガー


                    ret = 1;
                }
            }
            break;
			case WM_LBUTTONDBLCLK:
			{
				this->DblClickSelection();


			}
			break;
            
	    }
    }
    return ret;
}

//----------------------------------------------------------------
// 
//
//
//----------------------------------------------------------------
void CTextEditorCtrl::DblClickSelection()
{
	int nSelStart = GetSelectionStart();
	int nSelEnd = GetSelectionEnd();


	// 後ろを検索
	WCHAR cb[256];
	ct_->GetText(nSelEnd, cb, 256);

	WCHAR* p = cb;
	while(*p)
	{
		bool bl = (( '0' <= *p && *p <= '9' )||( 'A' <= *p && *p <= 'Z' )||( 'a' <= *p && *p <= 'z' )|| *p == '_');

		if ( !bl )
			break;

		nSelEnd++;
		p++;
	}

	//　前を検索
	memset(cb,0,sizeof(WCHAR)*256);
	ct_->GetText(max(0,nSelStart-256), cb, 256);
	p = cb;	

	int pr= nSelStart;
	p += min(nSelStart,256-1);
	while(nSelStart>0)
	{
		bool bl = (( '0' <= *p && *p <= '9' )||( 'A' <= *p && *p <= 'Z' )||( 'a' <= *p && *p <= 'z' )|| *p == '_');
		if ( !bl )
			break;

		nSelStart--;
		p--;
	}

	MoveSelection(nSelStart, nSelEnd,true);


}
//----------------------------------------------------------------
// 
//
//
//----------------------------------------------------------------

BOOL CTextEditorCtrl::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    BOOL ret = true; 

	bool pushShift   = (GetKeyState(VK_SHIFT)& 0x80) != 0;

	int nSelStart;
    int nSelEnd;

    static int first_x = 0;
    switch (0xff & wParam)
    {
        case VK_RETURN:
        {
            this->rebuild_ = true;

            
            
        }
        break;
        case VK_LEFT:
        {           
             if (pushShift)
             {                 				 
				 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();
                 if (nSelStart > -1)
                 {					
					if ( nSelStart == nSelEnd )
						ct_->bSelTrail_ = false;

					if ( ct_->bSelTrail_ )
						MoveSelection(nSelStart, nSelEnd-1, true );
					else
						MoveSelection(nSelStart - 1, nSelEnd, false);											
                 }
             }
             else
             {
                 MoveSelectionPrev();
             }

             auto rc = layout_.RowCol(CurrentCaretPos());

             first_x = rc.col;
             search_x_ = 0;
        }
		break;

        case VK_RIGHT:
        {
             if (pushShift)
             {
                 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();

				 if ( nSelStart == nSelEnd )
						ct_->bSelTrail_ = true;

				if ( ct_->bSelTrail_ )
					MoveSelection(nSelStart, nSelEnd + 1,true);
				else
					MoveSelection(nSelStart+1, nSelEnd,false);
             }
             else
             {
                 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();

                 MoveSelectionNext();
             }

             auto rc = layout_.RowCol(CurrentCaretPos());

             first_x = rc.col;
             search_x_ = 0;
        }
		break;

        case VK_UP:
             ret = MoveSelectionUpDown(TRUE, pushShift, first_x);
		break;

        case VK_DOWN:
             ret = MoveSelectionUpDown(FALSE, pushShift, first_x);
		break;

        case VK_HOME:
        {
            first_x = 0;
            UINT pos = MoveSelectionToLineFirstEnd(TRUE, pushShift);

            AdjustHScrollbar();
            ret = true;
        }
		break;

        case VK_END:
        {
             UINT pos = MoveSelectionToLineFirstEnd(FALSE, pushShift);
             AdjustHScrollbar();

             ret = true;
        }           
		break;

        case VK_DELETE:
			nSelStart = GetSelectionStart();
			nSelEnd = GetSelectionEnd();
			if (nSelStart == nSelEnd)
			{
				DeleteAtSelection(FALSE);
			}
			else
			{
				DeleteSelection();
			}

            this->rebuild_ = true;
             
		break;

        case VK_BACK:
             nSelStart = GetSelectionStart();
             nSelEnd = GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 DeleteAtSelection(TRUE);
             }
             else
             {
                 DeleteSelection();
             }

             this->rebuild_ = true;
             
		break;
		case VK_ESCAPE:
			nSelEnd = GetSelectionEnd();
			MoveSelection(nSelEnd, nSelEnd);
		break;
        case VK_PRIOR: // PAGE UP
        {
            int offy = 2*(int)(view_sz_.cy / layout_.GetLineHeight());
            
            MoveSelectionUpDown(TRUE, pushShift, first_x, offy);

        }
        break;
        case VK_NEXT: // PAGE DOWN
        {
            int offy = 2*(int)(view_sz_.cy / layout_.GetLineHeight());
            MoveSelectionUpDown(FALSE, pushShift, first_x, offy);


        }
        break;
        case VK_F2:
        {

            ret = 1;
        }
        break;
        
            
    }



	return ret;
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    //SetFocus();
}

//----------------------------------------------------------------
//
// vscroll bar
//
//----------------------------------------------------------------

bool CTextEditor::ScrollByWheel(bool bup)
{
    _ASSERT(GetLineHeight() != 0);


    CSize sz = view_sz_;
    int d = (ct_->IsShowHScrollbar() ? SCROLLBAR_WIDTH : 0);
    sz.cy -= d;
    ///////////////////////////////////////    
    CSize item_sz_(0,  GetLineHeight());
    CRect vscbar_rc = ct_->vscbar_rc_;
    int& top_idx_ = ct_->top_row_idx_;

    int cnt = RowCount();
    int total_h = item_sz_.cy * cnt;
    int af = total_h - sz.cy;
    int afr = af / item_sz_.cy;
    int off = (bup ? -1 : 1);
    top_idx_ = max(0, min(top_idx_ + off, afr+1));

    
    // set Scrollbar position
    int vc = top_idx_ * item_sz_.cy;
    float vh = (float)sz.cy;
    float nvh = item_sz_.cy * cnt - vh;

    //float b = vh - vscbar_rc.Height();
    //float c = (float)vscbar_rc.top;
    //int h = vscbar_rc.Height();


    float thumb_height = max(50, min(vh, (vh * vh) / (nvh + vh)));
    float b = vh - thumb_height;

    vscbar_rc.top = (int)(vc * b / nvh);
    vscbar_rc.bottom = vscbar_rc.top + thumb_height;
    
    ct_->vscbar_rc_ = vscbar_rc;
    return true;
}
void CTextEditor::ScrollbarRowoff(int off_row)
{
    _ASSERT(GetLineHeight()!=0);

    ct_->top_row_idx_ = max(0, min(ct_->top_row_idx_ + off_row, ((int)RowCount() - (int)(view_sz_.cy / GetLineHeight()))));
    
    CSize sz = view_sz_;
    int d = (ct_->IsShowHScrollbar() ? SCROLLBAR_WIDTH : 0);
    sz.cy -= d;
    ///////////////////////////////////////
    CSize item_sz_(0, GetLineHeight());
    
    CRect vscbar_rc = ct_->vscbar_rc_;
    int& top_idx_ = ct_->top_row_idx_;
    int cnt = RowCount(); 

    // set Scrollbar position
    int vc = top_idx_ * item_sz_.cy;
    float vh = (float)sz.cy;
    float nvh = item_sz_.cy * cnt - vh;

    //float b = vh - vscbar_rc.Height();
    //float c = (float)vscbar_rc.top;
    
    //int h = vscbar_rc.Height();

    float thumb_height = max(50, min(vh, (vh * vh) / (nvh + vh)));
    float b = vh - thumb_height;

    vscbar_rc.top = (int)(vc * b / nvh);
    vscbar_rc.bottom = vscbar_rc.top + thumb_height;

    ct_->vscbar_rc_ = vscbar_rc;
}
void CTextEditor::ScrollbarYoff(int scrollbar_off_y)
{
    if (scrollbar_off_y == 0) return;

    CSize item_sz_(0, GetLineHeight());
    CSize sz = view_sz_;
    int d = (ct_->IsShowHScrollbar() ? SCROLLBAR_WIDTH : 0);
    sz.cy -= d;


    int& top_idx_ = ct_->top_row_idx_;
    int cnt = RowCount();
    int offy = scrollbar_off_y;

    CRect rc = ct_->vscbar_rc_;

    //////////////////////////////

    rc.OffsetRect(0, offy);

    if (rc.bottom > sz.cy)
    {
        offy = sz.cy - rc.bottom;
        rc.OffsetRect(0, offy);
    }


    bool bl = true;
    if (rc.top < 0)
        bl = false;
    if (rc.bottom-d > sz.cy)
        bl = false;

    if (bl)
        ct_->vscbar_rc_ = rc;

    float vh = (float)sz.cy;
    float nvh = item_sz_.cy * cnt - vh;

    float b = vh - ct_->vscbar_rc_.Height();
    float c = (float)ct_->vscbar_rc_.top;
    float vc = (c * nvh) / b;

    //offy = (int)(vc + 0.5);
    top_idx_ = (int)max(0, min((int)(vc / item_sz_.cy), cnt));
}


CRect CTextEditor::InitVScollbar(CSize viewsz, int rowcount, int item_height)
{
    CRect scrollbar;

    int view_height = viewsz.cy;
    float vh = (float)view_height;
    float nvh = item_height * (rowcount) - vh;

    float thumb_height = max(50, min(vh, (vh * vh) / (nvh + vh)));

    scrollbar.SetRect(viewsz.cx - SCROLLBAR_WIDTH, 0, viewsz.cx, (int)thumb_height);
    return scrollbar;
}
//----------------------------------------------------------------
//
// hscroll bar
//
//----------------------------------------------------------------
void CTextEditor::ScrollbarXoff(int scrollbar_off_x)
{
    if (scrollbar_off_x == 0) return;

    CSize sz = view_sz_;

    int d = (ct_->IsShowVScrollbar() ? SCROLLBAR_WIDTH : 0);

    sz.cx -= d;

    int offx = scrollbar_off_x;

    CRect rc = ct_->hscbar_rc_;

    int max_width = (int)ct_->line_width_max_;

    //////////////////////////////
    rc.OffsetRect(offx, 0);
    if (rc.right > sz.cx)
    {
        offx = sz.cx- rc.right;
        rc.OffsetRect(offx, 0);
    }

    bool bl = true;
    if (rc.left < 0)
        bl = false;
    if (rc.right > sz.cx+d)
        bl = false;

    if (bl)
        ct_->hscbar_rc_ = rc;

    float vw = (float)sz.cx;
    float nvh = max_width - vw;

    float b = vw - ct_->hscbar_rc_.Width();
    float c = (float)ct_->hscbar_rc_.left;
    float vc = (c * nvh) / b;

    ct_->scrollbar_offx_ = (int)max(0, min(vc, view_sz_.cx));
}

// Align the h_scrollbar with the current caret.
void CTextEditor::AdjustHScrollbar()
{
    auto cur_RC = layout_.RowCol(CurrentCaretPos());
    auto drc = layout_.char_rects_.RowRect(cur_RC.row, cur_RC.col, false);

    CRect hscbar_rc = ct_->hscbar_rc_;

    int offx = drc.left- hscbar_rc.left;

    if (cur_RC.col!=0 )
    {
        if (drc.right < hscbar_rc.right)
            return;

        offx = view_sz_.cx - hscbar_rc.Width()- hscbar_rc.left;
    }
    ScrollbarXoff(offx);
}

CRect CTextEditor::InitHScollbar(CSize viewsz, int rowcount, int item_height)
{
    CRect scrollbar;
    int max_width = (int)ct_->line_width_max_;

    int d = (ct_->IsShowVScrollbar() ? SCROLLBAR_WIDTH : 0);
    int view_height = viewsz.cx-d;
    float vh = (float)view_height;
    float nvh = max_width - vh;

    float a = max(50, min(vh, (vh * vh) / (nvh + vh)));

    scrollbar.SetRect( 0, viewsz.cy - SCROLLBAR_WIDTH, (int)a, viewsz.cy);
    return scrollbar;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::Undo()
{
	auto b = ct_->Undo();

	if ( b.len < 0 )
		return;

	if ( b.p == nullptr )
	{
		// cancel insert

		ct_->RemoveText(b.caretpos, b.len, false);

		auto pos = b.caretpos;
		MoveSelection(pos, pos );
	}
	else
	{
		// cancel delete

		std::wstring deleted_string = (LPCWSTR)b.p.get();

		UINT res;
		ct_->InsertText(b.caretpos,deleted_string.c_str(),b.len, res, false );

		auto pos = b.caretpos+b.len;
		
		MoveSelection(pos, pos );
	}
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonDown(float x, float y)
{
    CPoint pt((int)x, (int)y);
    SelDragStart_ = -1;

    if (MoveSelectionAtPoint(pt))
    {
        SelDragStart_ = GetSelectionStart();
    }
	else
	{
		int end = ct_->SelEnd();
		MoveSelection( end, end, true);
	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonUp(float x, float y)
{
    int nSelStart = GetSelectionStart();
    int nSelEnd = GetSelectionEnd();
    CPoint pt((int)x, (int)y);

    if (MoveSelectionAtPoint(pt) && SelDragStart_ != -1)
    {
        int nNewSelStart = nSelStart;
        int nNewSelEnd = nSelEnd;

		bool bl = true;
			if ( nNewSelStart < SelDragStart_)
				bl = false;

        if ( bl )
        {
            nSelStart = nNewSelStart;
            nSelEnd = nNewSelStart;
        }

        MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd),bl); 


        SelDragStart_ = GetSelectionStart();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnMouseMove(float x, float y, bool bLbutton)
{   
    if( bLbutton && SelDragStart_ != -1)
    {
        CPoint pt((int)x, (int)y);

        int nSel = (int)layout_.CharPosFromPoint(pt);

        if (MoveSelectionAtPoint(pt))
        {
            int nNewSelStart = GetSelectionStart();
            int nNewSelEnd = GetSelectionEnd();

			auto bl = true;
			if ( nNewSelStart < SelDragStart_)
				bl = false;

            MoveSelection(min(SelDragStart_, nNewSelStart), max(SelDragStart_, nNewSelEnd), bl); 
        }
    }
}

bool CTextEditorCtrl::CopyBitmap(CBitmap* dstbmp)
{
	if (dstbmp == NULL )
		return false;

    BITMAP bmpInfo;
    if (!bmpText_.GetBitmap(&bmpInfo))
        return false;

    
     int cx = bmpInfo.bmWidth;
     int cy = bmpInfo.bmHeight;
    

	CDC cDC;
	cDC.CreateCompatibleDC(NULL);
	if (!cDC.GetSafeHdc())
		return false;
	CBitmap* oldbmp = cDC.SelectObject(&bmpText_);
	if (!oldbmp)
		return false;
	dstbmp->DeleteObject();
	dstbmp->CreateCompatibleBitmap(&cDC, cx, cy);
	if (!dstbmp->GetSafeHandle())
	{
		cDC.SelectObject(oldbmp);
		return false;
	}
	CDC dstDC;
	dstDC.CreateCompatibleDC(&cDC);
	dstDC.SelectObject(dstbmp);
	dstDC.BitBlt(0, 0, cx, cy, &cDC, 0, 0, SRCCOPY);
	cDC.SelectObject(oldbmp);
	return true;



}


#endif

//////////////////////////////////////////////////////////////////////////////
//
// CTextEditSink
//
//////////////////////////////////////////////////////////////////////////////
#include "DisplayAttribute.h"
#pragma region IUnknown
//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}
#pragma endregion
//+---------------------------------------------------------------------------
//
// CTextEditSink ctor
//
//----------------------------------------------------------------------------

CTextEditSink::CTextEditSink(CTextEditor *pEditor)
{
    _cRef = 1;
    _dwEditCookie = TES_INVALID_COOKIE;
    _pEditor = pEditor;
}

//+---------------------------------------------------------------------------
//
// EndEdit
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
#ifdef _WINDOWS
    CDispAttrProps *pDispAttrProps = GetDispAttrProps();
    if (pDispAttrProps)
    {
        IEnumTfRanges *pEnum;
        if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                             pDispAttrProps->GetPropTablePointer(),
                                                             pDispAttrProps->Count(),
                                                             &pEnum)) && pEnum)
        {
            ITfRange *pRange;
            if (pEnum->Next(1, &pRange, NULL) == S_OK)
            {
                // We check if there is a range to be changed.
                pRange->Release();

                _pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                ITfRange *pRangeEntire = NULL;
                ITfRange *pRangeEnd = NULL;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    IEnumTfRanges *pEnumRanges;
                    ITfReadOnlyProperty *pProp = NULL;

                    GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire)))
                    {
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE da;
                            TfGuidAtom guid;
                            if (GetDisplayAttributeData(ecReadOnly, pProp, pRange, &da, &guid) == S_OK)
                            {
                                ITfRangeACP *pRangeACP;
                                if (pRange->QueryInterface(IID_ITfRangeACP, (void **)&pRangeACP) == S_OK)
                                {
                                    LONG nStart;
                                    LONG nEnd;
                                    pRangeACP->GetExtent(&nStart, &nEnd);
                                    
									
									_pEditor->AddCompositionRenderInfo(nStart, nStart + nEnd, &da);


                                    pRangeACP->Release();
                                }
                            }
                        }
                    }
                }

                if (pRangeEntire)
                    pRangeEntire->Release();
                if (pRangeEnd)
                    pRangeEnd->Release();
 
            }
            pEnum->Release();
        }

        delete pDispAttrProps;



        //if (OnChanged_)
        //    OnChanged_();
    }
#endif
	

	

    return S_OK;
}





#pragma region Advise_Unadvice
//+---------------------------------------------------------------------------
//
// CTextEditSink::Advise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwEditCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    if (source)
        source->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// CTextEditSink::Unadvise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        goto Exit;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    if (source)
        source->Release();

    if (_pic)
    {
        _pic->Release();
        _pic = NULL;
    }

    return hr;
}
#pragma endregion