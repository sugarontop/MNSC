#include "pch.h"
#include "TextLayout.h"

using namespace TSF;

CTextLayout::CTextLayout()
{
	nLineCnt_ = 0;
	str_ = NULL;
	row_width_ = 0;
	bPassword_ = false;
	bRecalc_ = true;
	
	//selected_halftone_color_ = D2RGBA(0,140,255,100);
}
CTextLayout::~CTextLayout()
{
	Clear();
}

//----------------------------------------------------------------
//
//
// Calc layout 行数の把握と文字単位にPOS,LEN,RECTを取得
//----------------------------------------------------------------
BOOL CTextLayout::CreateLayout(CDC& cDC, const WCHAR* psz, int nCnt, const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos)
{
	Clear();
	bSingleLine_ = bSingleLine;

	str_ = psz;
	nLineCnt_ = 0;
	StarCharPos_ = 0;
	// 行数を計算
	{
		BOOL bNewLine = TRUE;
		for (int i = 0; i < nCnt; i++)
		{
			switch (psz[i])
			{
			case 0x0d:
			case 0x0a:
				if (bNewLine)
					nLineCnt_++;
				bNewLine = TRUE;
				break;
			default:
				if (bNewLine)
					nLineCnt_++;
				bNewLine = FALSE;
				break;
			}
		}
	}

	// Count character of each line.　文字単位にPOS,LEN,RECTを取得

	int nCurrentLine = 0;

	CHPOS c;
	UINT crlf = 0;
	int col = 0;
	{

		int nNewLine = 0;

		for (int i = 0; i < nCnt; i++)
		{
			switch (psz[i])
			{
			case 0x0d:
			case 0x0a:
			{
				c.pos = i;
				c.col = col;
				c.row = nCurrentLine;				
				c.lf = true;
				CharPosMap_.push_back(c);

				nNewLine = 0;
				nCurrentLine++;
				col = -1;

				_ASSERT(crlf == 0 || crlf == psz[i]); // CRLF is NG.  CRCR.. or LFLF.. is OK
				crlf = psz[i];
			}
			break;
			default:
				/*if (nNewLine)
				{
					nCurrentLine++;
					col=0;
				}*/

				c.pos = i;
				c.col = col;
				c.row = nCurrentLine;
				c.lf = false;

				CharPosMap_.push_back(c);

				nNewLine = 0;
				crlf = 0;
				break;
			}
			col++;
		}
	}

	{
		//		CreateTextLayout ct(cxt.tsf_wfactory_, psz, nCnt, fmt, sz, bSingleLine_);

		//		StarCharPos = ct.CreateDWriteTextLayout( StarCharPos, zCaret, &DWTextLayout_ ); //★TextLayout_

		//		_ASSERT(DWTextLayout_);

		StarCharPos_ = StarCharPos;

		//char_rectf_.Set(DWTextLayout_, false);

		int slen = nCnt;
		int len = 0;

		//=============================================
		// 文字文のRECT作成、取得
		//
		const RECT* prcs = char_rects_.Create(cDC, psz, sz, slen, &len);

		nLineHeight_ = char_rects_.LineHeight();

		for (int rcidx = 0; rcidx < len; rcidx++)
		{
			RECT rc = prcs[rcidx];
			CharPosMap_[rcidx].rc = rc;
		}


		//DWTextLayout_->GetMetrics(&tm_);
		return TRUE;
	}


	return FALSE;
}

float CTextLayout::GetLineHeight() const
{
	return nLineHeight_;
}


BOOL CTextLayout::Draw(CDC& cDC, const FRectF& rcText, LPCWSTR psz, int nCnt, int nSelStart, int nSelEnd, bool bTrail, int CaretPos)
{
	cDC.OffsetViewportOrg(rcText.left, rcText.top);
	cDC.SetBkMode(TRANSPARENT);


	if (!CharPosMap_.empty() && nSelEnd != 0)
	{
		CBrush br2(RGB(200, 200, 200));
		for (int j = nSelStart; j < nSelEnd; j++)
		{
			CRect rc3 = CharPosMap_[j].rc;
			cDC.FillRect(&rc3, &br2);
		}
	}
	
	
	CRect rc(0, 0, rcText.Width(), rcText.Height());

	cDC.DrawTextW(const_cast<LPWSTR>(psz), nCnt, &rc, (int)DT_TOP|DT_LEFT); //| DT_NOPREFIX | DT_SINGLELINE | DT_NOCLIP



	


	//for (int j = 0; j < nCnt; j++)	
	//{
	//	CRect rck = CharPosMap_[j].rc;//.GetRECT();
	//	
	//	cDC.FrameRect(rck, &br);
	//}
	
	
	

	


	cDC.OffsetViewportOrg(-rcText.left, -rcText.top);
	return TRUE;
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::RectFromCharPos(UINT nPos, CRect *prc)
{
	return RectFromCharPosEx( (int)nPos, -1, prc, nullptr );
 }

BOOL CTextLayout::RectFromCharPosEx(int nPos,int alignment, CRect*prc, bool* blf)
{
	if ( nPos < 0 )
	{		
		if ( !CharPosMap_.empty())
		{
			*prc = CharPosMap_[0].rc;
			prc->right=prc->left + TSF_FIRST_POS;
		}
		else if (alignment == 0 || alignment == -1) // left
		{
			prc->left = 0;
			prc->bottom = prc->top + nLineHeight_; 
			prc->right=TSF_FIRST_POS;
		}
		else if (alignment == 1) // center
		{
			prc->left = (prc->left+prc->right)/2;
			prc->bottom = prc->top + nLineHeight_; 
			prc->right = prc->left+ TSF_FIRST_POS;
		}
		else if ( alignment == 2) // right
		{
			prc->left = prc->right - TSF_FIRST_POS;
			prc->bottom = prc->top + nLineHeight_; 		
		}
		return TRUE;
	}

	
	if ( this->nLineCnt_ == 0 ) return FALSE;

	if ( nPos < (int)CharPosMap_.size()) //find(nPos) != CharPosMap_.end())
	{
		auto c = CharPosMap_[nPos];
		CRect rc = c.rc;
		if ( c.lf )
		{			
			rc.left = 0;
			rc.right =TSF_FIRST_POS;
			rc.OffsetRect(0, rc.Height());		
			
			if (blf)
				*blf = true;
		}
		else
		{
			if (blf)
				*blf = false;
		}

		*prc = rc;
		return TRUE;
	}

	if ( nPos >= (int)CharPosMap_.size() )
	{
		// 文末
		if (blf) 
			*blf = true;

		auto c = CharPosMap_[CharPosMap_.size()-1];
		auto rc = c.rc ;

		if (c.lf )
		{
			rc.left = 0;
			rc.right =TSF_FIRST_POS;
			rc.OffsetRect(0, rc.Height());		
		}
		else
		{
			rc.left = rc.right;
		}
		*prc = rc;
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

int CTextLayout::CharPosFromPoint(const CPoint& pt)
{
	_ASSERT( StarCharPos_ == 0 || (StarCharPos_ && this->bSingleLine_) );
		
	int j = 0;
	int lastone = -1;
	for( auto& it : CharPosMap_ )
	{
		auto rc = it.rc;

		if ( it.lf )
		{
			rc.OffsetRect(0,rc.Height());
			rc.left = 0;
			rc.right = TSF_FIRST_POS;
		}

		if ( rc.top <= pt.y && pt.y <= rc.bottom )
		{
			lastone = j;

			if ( rc.PtInRect(pt) )
			{
				float w = rc.Width();
				if ( rc.left + (w*3/4) < pt.x )
					return j+1;

				return j;
			}
			else if ( j== 0 )
			{
				if ( pt.x < rc.left )
				{
					return 0;
				}
			}



		}
		else if ( lastone != -1 )
			break;


		j++;
	}
	
	if (lastone == -1 )
		lastone = (int)CharPosMap_.size();

	return lastone+1; //-1;
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
	_ASSERT(CharPosMap_.size() != 0 );

	auto c =  CharPosMap_[ min( uCurPos, (UINT)CharPosMap_.size()-1) ];

    if (bFirst)
	{
		// 先頭列へ
		UINT ret = min( uCurPos, (UINT)CharPosMap_.size()-1);
		
		while( 0 != ret )
		{
			auto c2 = CharPosMap_[ret];
			if ( c.row != c2.row )
			{
				ret++;
				break;
			}
			ret--;
		}
		return ret;

	}
	else
	{	
		// 行の最後尾
		UINT i = uCurPos;
		UINT ret=0;
		while( i < CharPosMap_.size() )
		{
			auto c2 = CharPosMap_[i];

			if ( c.row != c2.row || c2.lf )
				break;

			ret = i;
			i++;
		}
		return ret+1;

	}
    return (UINT)(-1);
}

//void CTextLayout::GetTextLayout( IDWriteTextLayout** ppt )
//{ 
//	*ppt = DWTextLayout_; 
//	DWTextLayout_->AddRef();
//}

//----------------------------------------------------------------
//
// memory clear
//
//----------------------------------------------------------------

void CTextLayout::Clear()
{
    nLineCnt_ = 0;

	/*if ( DWTextLayout_ )
	{
		DWTextLayout_->Release();
		DWTextLayout_ = nullptr;
	}*/


	CharPosMap_.clear();
}

