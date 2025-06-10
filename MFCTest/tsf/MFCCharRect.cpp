#include "pch.h"
#include "D2DMisc.h"
#include "CharRect.h"
#define TAB_WIDTH_4CHAR 4
#define TSF_FIRST_POS 1

using namespace V6;

CharsRect::CharsRect()
{	
	cnt_ = 0;
	rects_ = NULL;
	line_height_ = 0;
	bSingleLine_ = true;

}
CharsRect::CharsRect(bool bSingle)
{
	cnt_ = 0;
	rects_ = NULL;
	line_height_ = 0;
	bSingleLine_ = bSingle;

}

const RECT* CharsRect::Create(CDC& cDC, LPCWSTR str, const SIZE& sz, int slen, int* plen, int* lineHeight)
{
	int mkcnt_ = ((str == nullptr || slen == 0) ? 1 : slen);
	cnt_ = slen;
	rects_ = std::shared_ptr<RECT[]>(new RECT[mkcnt_]);
	ZeroMemory(rects_.get(), sizeof(RECT) * mkcnt_);

	CRect rc(0, 0, 0, sz.cy );
	
	CSize sz1;
	int k = 0;
	WORD ch_x = 0, ch_y = 0;
	for (int i = 0; i < slen; i++)
	{		
		GetTextExtentPoint32(cDC.GetSafeHdc(), str + k, (i - k) + 1, &sz1);

		rc.right = rc.left + (sz1.cx - rc.left);
		rc.bottom = rc.top + sz1.cy;

		if (str[i] == L'\n')
		{
			rc.left = rc.right;
			rects_[i] = rc;

			rc.left = rc.right = 0;
			rc.top += (rc.bottom - rc.top);

			ch_x = 0;
			ch_y++;
			k = i + 1;
		}
		else
		{
			rects_[i] = rc;

			line_height_ = rc.Height();

			rc.left = rc.right;
			ch_x++;
		}
	}
	*lineHeight = line_height_;
	*plen = slen;


	if (*lineHeight == 0)
	{
		GetTextExtentPoint32(cDC.GetSafeHdc(), L"T", 1, &sz1);
		*lineHeight = sz1.cy;
	}


	return rects_.get();
}

void CharsRect::Clear()
{
	cnt_ = 0;
	rects_ = nullptr;
	line_height_ = 0;
	bSingleLine_ = true;
}