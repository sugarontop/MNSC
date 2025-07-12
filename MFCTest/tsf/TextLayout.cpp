#include "pch.h"
#include "TextLayout.h"
#include <gdiplus.h>
using namespace Gdiplus;
using namespace TSF;

//#define __DRAW_ROW_RECT

CTextLayout::CTextLayout()
{
	nLineCnt_ = 0;
	str_ = NULL;
	row_width_ = 0;
	bPassword_ = false;
	bRecalc_ = true;
	
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
	char_rects_.Clear();

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
				nNewLine = 0;
				nCurrentLine++;
				col = -1;

				_ASSERT(crlf == 0 || crlf == psz[i]); // CRLF is NG.  CRCR.. or LFLF.. is OK
				crlf = psz[i];
			}
			break;
			default:
				
				nNewLine = 0;
				crlf = 0;
				break;
			}
			col++;
		}
	}

	{
		StarCharPos_ = StarCharPos;

		int slen = nCnt;
		int len = 0;
		
		//=============================================
		// 文字文のRECT作成、取得
		//
		int lineheight;
		TabWidth_ = 0;
		const std::vector <RowString>& prcs = char_rects_.Create(cDC, psz, slen, &TabWidth_ , &lineheight);

		nLineHeight_ = lineheight;

		int rcidx = 0;
		for(auto& r : prcs)
		{
			CRect krc;
			for(int i= 0; i < r.len; i++)
			{
				LEFTRIGHT lr = r.rects[i];
				CRect rc(lr.left, r.y, lr.right, r.y + r.cy);

				krc = rc;
			}
		}

		return TRUE;
	}


	return FALSE;
}

int CTextLayout::Row(int zPos)
{
	int r = 0;
	auto& ar = char_rects_.Get();

	int zpos = 0;
	for (auto& it : ar)
	{
		zpos += (it.len+1);

		if ( zPos < zpos )
			break;
		r++;
	}
	return r; // 0start
}
RC CTextLayout::RowCol(int zPos) const
{
	RC ret;
	int hp;
	ret.row = char_rects_.Row(zPos, &hp);
	ret.col = zPos - hp ;
	return ret;
}
int CTextLayout::ZPos(RC rc) const
{
	auto& ar = char_rects_.Get();

	int zpos = 0;
	int r = 0;
	for (auto& it : ar)
	{
		if ( r == rc.row )
		{
			const std::vector<RowString>& ar = char_rects_.Get();
			zpos += min (rc.col, ar[r].len);
			break;
		}

		zpos += (it.len + 1);
		r++;
	}
	return zpos;
}



BOOL CTextLayout::ReCreateLayout(CDC& cDC, const WCHAR* psz, int nCnt, const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos)
{
	if ( zCaret == 0 && nCnt == 0)
		return FALSE;

	if (zCaret == 0)
		return FALSE;

	int row = Row(zCaret-1);

	int nLineCnt = 0;
	const WCHAR* row_str = nullptr;
	int row_len = 0;
	{
		for (int i = 0; i < nCnt; i++)
		{
			row_len++;
			if ( row == nLineCnt && row_str == nullptr)
			{
				row_str = &psz[i];
				row_len = 1;
			}
			else if (row+1 == nLineCnt)
			{
				break;
			}

			switch (psz[i])
			{
				case 0x0d:
				case 0x0a:
					nLineCnt++;
				break;
			}
		}
	}

	if (row_str!=nullptr)
	{
		int lh;
		RowString rs;
		if ( char_rects_.CreateRow(cDC,row, row_str, row_len, &lh, TabWidth_, rs ))
		{
			char_rects_.Update(row, rs);
			return TRUE;
		}	
	}
	return TRUE;
}


float CTextLayout::GetLineHeight() const
{
	return nLineHeight_;
}

void CTextLayout::DrawSelectRange(CDC& cDC, const FRectF& rcText, int nSelStart, int nSelEnd)
{
	// 仕方なくGDI+を使う
	cDC.OffsetViewportOrg(rcText.left, rcText.top);

	if (!char_rects_.Get().empty() && nSelEnd != 0 && nSelEnd - nSelStart!=0)
	{		
		auto selarea = char_rects_.SerialRects(nSelStart, nSelEnd);
		
		
		Gdiplus::Graphics graphics(cDC.m_hDC);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		// 透過四角形の描画
		Gdiplus::SolidBrush brush(Color(100, 200, 100, 200)); 

		

		for(auto& rc3 : selarea )
		{
			//TRACE(L"%d, %d  %d-%d\n", rc3.left, rc3.right, nSelStart, nSelEnd);
			graphics.FillRectangle(&brush, (INT)rc3.left, (INT)rc3.top, (INT)(rc3.right-rc3.left), (INT)(rc3.bottom-rc3.top));
		}
	}

	cDC.OffsetViewportOrg(-rcText.left, -rcText.top);
}


float CTextLayout::TabWidth() const
{
	return TabWidth_;
}


BOOL CTextLayout::Draw(CDC& cDC, int start_row, const FRectF& rcText, LPCWSTR psz, int nCnt, int nSelStart, int nSelEnd, bool bTrail, int CaretPos)
{
	_ASSERT(rcText.left==0 && rcText.top == 0);
	
	auto& rows = char_rects_.Get();

	int end_row = (int)rows.size();

	offsetPt_ = CPoint(0, start_row * nLineHeight_);

	cDC.OffsetViewportOrg(-offsetPt_.x, -offsetPt_.y);
	
	int tabStops = TabWidth();
	for(int row = start_row; row < end_row; row++ )
	{
		auto& ir = rows[row];
		CRect rc(0,ir.y, ir.cx, ir.y+ir.cy);
		//cDC.DrawTextW(const_cast<LPWSTR>(ir.str.c_str()), ir.len, &rc, DT_TOP | DT_LEFT| DT_EXPANDTABS);
		cDC.TabbedTextOut(rc.left, rc.top, ir.str.c_str(), ir.len, 1, &tabStops, rc.left);
	}

	

	#ifdef __DRAW_ROW_RECT
	CBrush br(RGB(0,0,0));
	int jr = 0;
	for(auto& r : char_rects_.Get())
	{
		for(int col = 0; col < r.len+1; col++)
		{
			LEFTRIGHT lr = r.rects[col];
			CRect rck( lr.left, r.y, lr.right, r.y+r.cy);
			cDC.FrameRect(rck, &br);
		}

		jr++;
	}

	#endif

	cDC.OffsetViewportOrg(offsetPt_.x, offsetPt_.y);

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
	
	if ( 0 <= nPos )
	{
		auto& ar = char_rects_.Get();
		int j = 0;
		int pos = nPos;

		for(auto& it : ar)
		{
			if ( j <= nPos && nPos < j+it.len )
			{
				_ASSERT( 0 <= pos && pos < it.len );

				LEFTRIGHT lr = it.rects[pos];
				CRect rc(lr.left, it.y, lr.right, it.y + it.cy);
				
				*prc = rc;
				return TRUE;
			}
			else if ( nPos < j+it.len+1 )
			{
				CRect rc(0, it.y+it.cy, 1, it.y + it.cy*2);

				*prc = rc;
				return TRUE; // 空行の場合
			}

			j += (it.len + 1);
			pos -= (it.len+1);

		}
		

		return TRUE;
	}

	else if (nPos < 0)
	{
		prc->left = 0;
		prc->bottom = prc->top + nLineHeight_;
		prc->right = TSF_FIRST_POS;
		return TRUE;

	}

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

int CTextLayout::CharPosFromPoint(const CPoint& pt)
{
	int j = 0;
	bool bl = false;
	for (auto& r : char_rects_.Get())
	{
		for (int ic = 0; ic < r.len; ic++)
		{	
			LEFTRIGHT lr = r.rects[ic];
			CRect rc(lr.left, r.y, lr.right, r.y + r.cy);

			bl = false;
			if ( rc.top <= pt.y && pt.y <= rc.bottom )
			{			
				if ( rc.PtInRect(pt))
				{
					if (rc.left + (rc.Width() * 3 / 4) < pt.x)
					{
						return j+1;
					}
					return j;
				}
				j++;
				bl = true;
			}

			if ( bl )
			{
				lr = r.rects[r.len];
				if ( lr.right < pt.x )
  					return j+r.len-1;
			}			
		}

		if ( r.len == 0 )
			if (pt.y < r.y+r.cy)
				return j;

		j += r.len+1;
	}
	
	return j;

}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
	RC rc = RowCol(uCurPos);	
	rc.col = (bFirst? 0: 9999);
	return (UINT)ZPos(rc);
}

//----------------------------------------------------------------
//
// memory clear
//
//----------------------------------------------------------------
void CTextLayout::SetRecalc(bool bRecalc)
{ 
}
void CTextLayout::Clear()
{
	
    nLineCnt_ = 0;

}

