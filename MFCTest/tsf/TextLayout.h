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
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------


struct CHPOS
{
	CHPOS():pos(0),row(0),col(0),lf(false){};

	int pos;
	int row;
	int col;
	bool lf;
	//V6::FRectF rc;
	CRect rc;

};

CHPOS Real(const CHPOS& s) ;


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
		BOOL Draw(CDC& cDC, const FRectF& rcText, LPCWSTR psz, int nCnt, int nSelStart, int nSelEnd, bool bTrail, int CaretPos);
		BOOL CreateLayout(CDC& cDC, const WCHAR* psz, int nCnt, const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos);
	public :
		int CharPosFromPoint(const CPoint& pt);
		BOOL RectFromCharPos(UINT nPos, CRect *prc);
		BOOL RectFromCharPosEx(int nPos,int alignment, CRect*prc, bool* blf);

		UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

		float GetLineHeight() const;
		float GetLineWidth() const { return row_width_; }

		void Password( bool bl ){ bPassword_ = bl; }

		bool Recalc() const { return bRecalc_; }
		void SetRecalc(bool bRecalc) { bRecalc_ = bRecalc; }

		D2D1_COLOR_F selected_halftone_color_;
		void Clear();

		void test(){nLineHeight_ = 0;}

	private:
		std::vector<CHPOS> CharPosMap_;
		bool bRecalc_;
		
		
		UINT nLineCnt_;
		float nLineHeight_;
		LPCWSTR str_;
		FLOAT row_width_;
		bool bSingleLine_;
		bool bPassword_;
		int StarCharPos_;

		CharsRect char_rects_;
		
};

};

