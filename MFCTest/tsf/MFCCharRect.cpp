#include "pch.h"
#include "D2DMisc.h"
#include "CharRect.h"
#define TAB_WIDTH_4CHAR 4
#define TSF_FIRST_POS 1
#define LF_WIDTH 16
using namespace V6;

template <typename T> 
std::shared_ptr<T[]> ToArray(std::vector<T>& ar)
{
	// 配列のサイズが0の場合は空のshared_ptrを返す
	if (ar.empty()) {
		return std::shared_ptr<T[]>();
	}

	// 配列を確保し、vectorの内容をコピー
	auto ptr = std::shared_ptr<T[]>(new T[ar.size()]);
	std::copy(ar.begin(), ar.end(), ptr.get());

	return ptr;
}
void CharsRect::Update(int row, RowString& new_rowstr)
{
	if ( row < row_rects_.size())
	{
		new_rowstr.y = row_rects_[row].y;
		//_ASSERT(new_rowstr.cy == row_rects_[row].cy );

		row_rects_[row] = new_rowstr;
	}
}
std::vector<RECT> CharsRect::GetTextRects(int row, int* cnt) const
{
	*cnt = row_rects_[row].len;
	LEFTRIGHT* plr = row_rects_[row].rects.get();

	std::vector<RECT> ar(*cnt);
	int top = row_rects_[row].y;
	int bottom = top + row_rects_[row].cy;
	for(int i=0; i < *cnt; i++)
	{
		CRect rc( plr[i].left, top, plr[i].right, bottom );
		ar[i] = rc;
	}
	return ar;
}
bool CharsRect::CreateRow(CDC& cDC,int row, LPCWSTR str, int slen, int* lineHeight,int tabwidth, RowString& out)
{
	if (str != nullptr)
	{
		LEFTRIGHT lr;
		CRect rc(0,0,0,0);
		*lineHeight = 0;

		int is = 0;
		std::vector<WCHAR> strar;
		std::vector<LEFTRIGHT> lrar;

		for (int i = 0; i < slen+1; i++)
		{
			CSize sz1;
			const WCHAR* pch = (str+i);

			GetTextExtentPoint32(cDC.GetSafeHdc(), pch, 1, &sz1);

			if (sz1.cx == 0 && *pch == L'\n')
			{
				sz1.cx = LF_WIDTH; // LF char
			}
			else if (*pch == L'\t')
			{
				sz1.cx = tabwidth;
			}

			rc.right = rc.left + sz1.cx;
			rc.bottom = rc.top + sz1.cy;

			lrar.push_back( LEFTRIGHT(rc.left,rc.right));

			strar.push_back(*pch);

			rc.left = rc.right;


			if (str[i] == L'\n' || str[i] == 0 || i == slen)
			{
				RowString rs;
				
				rs.len = i - is; 
				rs.str = std::wstring(str+is, rs.len);		
				
				

				rs.rects = ToArray(lrar);
				rs.cy = rc.Height();
				rs.cx = rc.right;
				rs.y = rc.top;

				rc.left = 0;
				rc.top += rs.cy;

				is = i + 1;

				line_height_ = rs.cy;
				*lineHeight = line_height_;

				out = rs;
				return true;
			}
		}
	}
	return false;
}
const std::vector<RowString>& CharsRect::Create(CDC& cDC, LPCWSTR str, int slen, int* tabwidth, int* lineHeight)
{
	int space_width=0;
	//if (*lineHeight == 0)
	{
		CSize sz1;
		GetTextExtentPoint32(cDC.GetSafeHdc(), L" ", 1, &sz1);
		*lineHeight = sz1.cy;
		*tabwidth = sz1.cx * 4;
	}
	
	
	if (str != nullptr)
	{
		LEFTRIGHT lr;
		CRect rc(0, 0, 0, 0);
		*lineHeight = 0;

		int is = 0;
		std::vector<LEFTRIGHT> lrar;

		for (int i = 0; i < slen + 1; i++)
		{
			CSize sz1;

			

			GetTextExtentPoint32(cDC.GetSafeHdc(), str + i, 1, &sz1);

			if (sz1.cx == 0 && *(str+i) == L'\n')
			{
				sz1.cx = LF_WIDTH; // LF char
			}
			else if (*(str + i) == L'\t')
			{
				sz1.cx = *tabwidth;
			}

			rc.right = rc.left + sz1.cx;
			rc.bottom = rc.top + sz1.cy;

			lrar.push_back(LEFTRIGHT(rc.left, rc.right));
			rc.left = rc.right;

			if (str[i] == L'\n' || str[i] == 0 || i == slen)
			{
				RowString rs;
				rs.len = i - is;
				rs.str = std::wstring(str + is, rs.len);
				rs.rects = ToArray(lrar);
				rs.cy = rc.Height();
				rs.cx = rc.right;
				rs.y = rc.top;
				row_rects_.push_back(rs);
				
				rc.left = 0;
				rc.top += rs.cy;

				is = i + 1;

				line_height_ = rs.cy;

				lrar.clear();
				if (str[i] == 0)
					break;

			}
		}
		*lineHeight = line_height_;
	}
	return row_rects_;
}

int CharsRect::Row(int zPos, int* HeadzPos) const
{
	int row = 0;
	int z = 0;
	for(auto& it : row_rects_ )
	{
		if ( zPos < (it.len+1) )
		{
			if (HeadzPos)
				*HeadzPos = z;
			return row;
		}

		zPos -= (it.len+1);
		z += (it.len+1);
		row++;
	}
	return row;
}
std::vector<RECT> CharsRect::SerialRects(int zPos1, int zPos2) const
{
	std::vector<RECT> ret;
	
	int sPos = min(zPos1,zPos2);
	int ePos = max(zPos1, zPos2);

	int s1,e1;

	int sRow = Row(sPos, &s1);
	int eRow = Row(ePos, &e1);

	if ( sRow == eRow )
	{
		auto& it = row_rects_[sRow];
		auto& ar = it.rects;

		int idx1 = sPos - s1;
		int idx2 = ePos - e1;

		for(int i = idx1; i < idx2; i++ )
		{
			LEFTRIGHT lr = ar[i];
			CRect rc(lr.left, it.y, lr.right, it.y+it.cy);
			ret.push_back(rc);
		}
	}
	else
	{
		int idx1 = sPos - s1;
		auto rc1 = RowRect(sRow, idx1, true);
		ret.push_back(rc1);

		for(int k= sRow+1; k < eRow; k++ )
		{
			ret.push_back(RowRect(k,0,true));
		}
		
		int idx2 = ePos - e1;

		auto& it = row_rects_[eRow];
		auto& ar = it.rects;
		
		
		auto rc2 = RowRect(eRow, 0, true);
		LEFTRIGHT lr = ar[idx2];
		rc2.right = lr.right;
		ret.push_back(rc2);
	}

	return ret;
}

RECT CharsRect::RowRect(int row, int col, bool last) const
{
	_ASSERT(row< (int)row_rects_.size());

	auto& it = row_rects_[row];
	int len = it.len;
	auto& ar = it.rects;

	if ( col == 0 && len == 0)
	{
		return CRect(0,it.y, 0,it.y+it.cy); // LF char
	}

	col = min(col, len-1);
	_ASSERT(col < len);

	auto lr1 = ar[col];
	auto lr2 = lr1;

	if ( last )
		lr2 = ar[len];	

	CRect rc(lr1.left, it.y, lr2.right, it.y+it.cy);
	return rc;
		
}