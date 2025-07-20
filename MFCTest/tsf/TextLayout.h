#pragma once


#include "CharRect.h"
#include "msctf.h"

#define TSF_FIRST_POS 1

namespace TSF {

struct COMPOSITIONRENDERINFO 
{
    int nStart;
    int nEnd;
    TF_DISPLAYATTRIBUTE da;
};

struct RC
{
	int row;
	int col;

};

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
using namespace V6;

class CTextLayout
{
	friend class CTextEditor;
	public:
		CTextLayout();
		virtual ~CTextLayout();
				
		void DrawSelectRange(CDC& cDC, const FRectF& rcText, int nSelStart, int nSelEnd);
		BOOL Draw(CDC& cDC, int start_row, float view_height, float view_width, LPCWSTR psz, int nCnt, int nSelStart, int nSelEnd, bool bTrail, int CaretPos, int offx);
		BOOL CreateLayout(CDC& cDC, const WCHAR* psz, int nCnt, const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos);
		BOOL ReCreateLayout(CDC& cDC, const WCHAR* psz, int nCnt, const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos);
	public :
		int CharPosFromPoint(const CPoint& pt);
		BOOL RectFromCharPos(UINT nPos, CRect *prc);
		BOOL RectFromCharPosEx(int nPos,int alignment, CRect*prc, bool* blf);

		UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

		float GetLineHeight() const;
		float GetLineWidth() const { return row_width_;}

		float GetLineWidthMax() const { return char_rects_.LineWidthMax(); }

		void Password( bool bl ){ bPassword_ = bl; }

		bool Recalc() const { return bRecalc_; }
		void SetRecalc(bool bRecalc);

		D2D1_COLOR_F selected_halftone_color_;
		void Clear();
		int Row(int zPos);

		RC RowCol(int zPos) const;
		int ZPos( RC rc) const;

		CPoint Offset() const { return offsetPt_; }
		int RowCount() const { return (int)char_rects_.RowCount(); }

		float TabWidth() const;
	private:
		bool bRecalc_;
		int TabWidth_;

		UINT nLineCnt_;
		float nLineHeight_;
		LPCWSTR str_;
		FLOAT row_width_;
		bool bSingleLine_;
		bool bPassword_;
		int StarCharPos_;

		CPoint offsetPt_;

		CharsRect char_rects_;
		
};

};

