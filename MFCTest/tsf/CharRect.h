#pragma once
#include "D2DMisc.h"

namespace V6 {


/*
       start caret  end
0      zS    zC     zE            zEE 
+-------+--+--+--+--+--------------+------------------
 a| b| c| D| E/ F| G| h| i| j| k| l|
--------+--+--+--+--+--------------------------------
        0    rC     rE
--------+--+--+--+--+--------------------------------
        | view area |
--------+--+--+--+--+--------------------------------
*/

	// single line caret 位置計算
	class SingleLineCaret
	{
		public :
			SingleLineCaret( int zS, int zEE, int rE, int rC )
			{	
				rC = max(0,rC);
				zS = max(0,zS);

				_ASSERT( zS <= zEE );
				if ( rC <= rE )
				{
					zS_ = zS;
					zEE_ = zEE;
					rE_ = rE;
					rC_ = rC;
				}
				else
				{
					zS_ = zS + rC - rE;
					zEE_ = zEE;
					rE_ = rE;
					rC_ = rE;
				}
				_ASSERT( rC_ <= rE_ );
			}
		
			SingleLineCaret( LPCWSTR s,  int rE )
			{	
				zS_ = 0;
				zEE_ = (int)wcslen(s);
				rE_ = rE;
				rC_ = 0;
			}

			int CaretOff( int one )
			{
				_ASSERT( one == 1 || one == -1 || one == 0 );
				int rC = rC_ + one;
			
				if ( rC >= rE_ )
				{
					rC_ = rC - 3;				

					if ( zEE() - zC() < 4 )
					{
						rC_ = min( rE_, rC);
					}
					else
					{
						zS_ = zS_ + 3;
					}

				}
				else if ( rC <= 0 )
				{
					rC_ = rC + 3;

					if ( zC() < 4 )
					{
						rC_ = max(0, rC);
					}
					else
					{
						zS_ = zS_ - 3;
					}
				}
				else
					rC_ = rC;


				if ( zC() > zEE())
				{
					rC_ = zEE_-zS_;
				}
				else if ( zC() < 0 )
				{
					rC_ = 0;
				}

				zS_ = max(0, zS_ );

				return zS_;
			}


			// 文字の先頭オフセット位置
			int zS(){ return zS_; }

			// キャレットの絶対位置
			int zC(){ return zS_+rC_; }

			//表示文字の最後
			int zE(){ return zS_ + rE_; }

			// キャレットの相対位置、Textbox内での位置
			int rC(){ return rC_; }

			// 表示文字数
			int rE(){ return rE_; }

			// 文字の長さ
			int zEE(){ return zEE_; }

		private :
			int zS_,zEE_;
			int rC_,rE_;

		public :
	

	};

	struct LEFTRIGHT
	{
		LEFTRIGHT():left(0),right(0){}
		LEFTRIGHT(int l,int r):left(l),right(r){}
		int left;
		int right;
	};

	struct RowString
	{
		RowString():len(0),y(0),cx(0),cy(0){}

		std::wstring str;
		int len; // 行の長さ LF含まず
		int y;  // 行のy
		int cy; // 行の高さ
		int cx;	 // 行の全文字幅
		std::shared_ptr<LEFTRIGHT[]> rects;
		
	};

	class CharsRect
	{
		public:
			CharsRect() :line_height_(0), line_width_max_(0){};
			~CharsRect() { Clear(); }

			const std::vector <RowString>& Create(CDC& cDC, LPCWSTR str, int slen,int* tabwidth, int* lineHeight);

			bool CreateRow(CDC& cDC, int row, LPCWSTR str, int slen, int* lineHeight, int tabwidth, RowString& out);

			void Clear() { row_rects_.clear(); line_height_ = 0; }
			float LineHeight() const { return line_height_; }
			float LineWidthMax() const { return line_width_max_; }
			
			int RowCount() const { return (int)row_rects_.size(); }
			std::vector<RECT> GetTextRects(int row, int* cnt) const;

			const std::vector <RowString>& Get() const { return row_rects_; }

			void Update(int row, RowString& rowstr);
			bool empty() const { return row_rects_.empty(); }

			
			int Row(int zPos, int* HeadzPos) const;
			std::vector<RECT> SerialRects(int zPos1, int zPos2) const;
			RECT RowRect(int row, int col, bool last) const;
			


		private:
			std::vector<RowString> row_rects_;
			float line_height_;
			float line_width_max_;

	};


/*
 
RowString it = ...;

CRect krc(0,it.y ,it.cx ,it.y+it.cy);

int xx = it.len;

dc.DrawText(it.str.c_str(), it.len, &krc, DT_LEFT);

*/

};
