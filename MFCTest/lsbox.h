#pragma once

#define SCROLLBAR_WIDTH	20
class ListboxBase
{
	public :		
		ListboxBase():top_idx_(0),select_idx_(-1){};

	protected :
		CSize sz_, item_sz_;
		CRect vscbar_rc_;
		std::vector<std::wstring> ar_;
		int top_idx_;
		int select_idx_;
	public :
		void Draw(CDC* pDC)
		{
			CFont* pf = pDC->GetCurrentFont(); 

			CBitmap bmp;
			CDC memDC; 
			bmp.CreateCompatibleBitmap(pDC, sz_.cx,sz_.cy);			
			memDC.CreateCompatibleDC(pDC);
			CBitmap* oldb = memDC.SelectObject(&bmp);
			CFont* oldf = memDC.SelectObject(pf);

			CBrush br;
			br.Attach((HBRUSH)::GetStockObject(BLACK_BRUSH));

			CRect rc2,rc;
			rc2.SetRect(0, 0, sz_.cx, sz_.cy);
			memDC.FillSolidRect(rc2, RGB(230, 230, 230));
			
			rc.SetRect(0, 0, sz_.cx- SCROLLBAR_WIDTH, sz_.cy);
			memDC.FillSolidRect(rc, RGB(255, 255, 255));

			memDC.Rectangle(rc2);

			memDC.SaveDC();
			memDC.SetMapMode(MM_TEXT);
			memDC.SetBkMode(TRANSPARENT);

			

			rc.SetRect(0,0, item_sz_.cx- SCROLLBAR_WIDTH, item_sz_.cy);
			int h = 0;
			for (int i = top_idx_; i < (int)ar_.size(); i++)
			{
				if ( i == select_idx_ )
					memDC.FillSolidRect(rc, RGB(200, 200, 220));

				const std::wstring& str = ar_[i];
				memDC.DrawText(str.c_str(), (int)str.length(), rc, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
				
				memDC.OffsetViewportOrg(0, rc.Height());
				h += rc.Height();

				if ( (h+ rc.Height()) > sz_.cy )
					break;
			}
			memDC.RestoreDC(-1);

			memDC.FillSolidRect(vscbar_rc_, RGB(120, 120, 120));

			memDC.SelectObject(oldf);

			pDC->BitBlt(0, 0, sz_.cx, sz_.cy, &memDC, 0, 0, SRCCOPY);
			memDC.SelectObject(oldb);

			
		}
		int SelectRowTest( CPoint point, bool bLast=false)
		{
			if (point.y < 0)
				return 0;

			// �擪�s��point.y=0
			if ( 0 <point.x && point.x < sz_.cx- SCROLLBAR_WIDTH && 0 < point.y && point.y < sz_.cy)
			{
				int r = point.y / item_sz_.cy;

				if ( r + top_idx_ < (int)ar_.size())
				{
					if (bLast)
						SetSelect( r+top_idx_);
					return 1; // row
				}
			}			
			else if (sz_.cx - SCROLLBAR_WIDTH< point.x && point.x < sz_.cx)
			{
				if (point.y < sz_.cy)
					return 2; // scrollbar
			}
			return 0;
		}

		void SetViewSize(CSize sz, CSize itemsz)
		{
			sz_ = sz;
			item_sz_ = itemsz;

		}
		void SetString(const std::vector<std::wstring>& ar)
		{
			ar_ = ar;

			top_idx_=0;
			select_idx_ = -1;

			float vh = (float)sz_.cy;
			float nvh = item_sz_.cy * ar.size() - vh;

			float a = max(50, min(vh, (vh * vh) / (nvh + vh)));

			vscbar_rc_.SetRect(sz_.cx - SCROLLBAR_WIDTH, 0, sz_.cx, (int)a );
		}
		void ScrollbarYoff(int offy)
		{
			if ( offy == 0 ) return;

			CRect rc = vscbar_rc_;

			rc.OffsetRect(0, offy);
			bool bl = true;
			if (rc.top < 0)
				bl = false;
			if (rc.bottom > sz_.cy)
				bl = false;

			if (bl)
				vscbar_rc_ = rc;




			float vh = (float)sz_.cy;
			float nvh = item_sz_.cy * ar_.size() - vh;

			float b = vh - vscbar_rc_.Height();
			float c = (float)vscbar_rc_.top;
			float vc = (c * nvh) / b;

			offy = (int)(vc + 0.5);
			top_idx_ = (int)max(0, min((int)(vc / item_sz_.cy), (int)ar_.size()));
		}
		int GetSelectIdx() const 
		{ 
			return select_idx_; 
		}
		std::wstring GetSelectString() const
		{
			if ( select_idx_ < 0 )
				return L"";

			return ar_[select_idx_];
		}
		void SetSelect(int idx)
		{ 
			select_idx_= max( -1, idx); 
		}
		CSize Size() const
		{			
			return sz_;
		}
		bool ScrollByMoude(bool bdown)
		{			
			int cnt = (int)ar_.size();
			int total_h = item_sz_.cy * cnt;
			int af = total_h - sz_.cy;
			int afr = af / item_sz_.cy;
			int off = (bdown ? -1 : 1);
			top_idx_ = max(0, min(top_idx_+off, afr));

			// set Scrollbar position
			int vc = top_idx_* item_sz_.cy;
			float vh = (float)sz_.cy;
			float nvh = item_sz_.cy * ar_.size() - vh;

			float b = vh - vscbar_rc_.Height();
			float c = (float)vscbar_rc_.top;

			int h = vscbar_rc_.Height();
			vscbar_rc_.top = (int)(vc*b/nvh);
			vscbar_rc_.bottom = vscbar_rc_.top + h;

			return true;
		}


};