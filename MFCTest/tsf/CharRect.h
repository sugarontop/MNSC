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

	class CharsRect
	{
	public:
		CharsRect();
		CharsRect(bool bSingle);
		~CharsRect() { Clear(); }

		const RECT* Create(CDC& cDC,LPCWSTR str, const SIZE& sz, int slen, int* plen);

		void Clear();
		float LineHeight() const { return line_height_; }

		const RECT* GetTextRects() const { return rects_.get(); }
	private:
		std::shared_ptr<RECT[]> rects_;

		bool bSingleLine_;
		int cnt_;
		float line_height_;
	};



};
