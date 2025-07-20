#pragma once

namespace TSF {

class UndoTextEditor
{
	public :
		UndoTextEditor(){ };

		struct BInfo
		{
			BInfo():len(-1),caretpos(0),stat(0){}

			std::shared_ptr<WCHAR[]> p;
			int len;
			UINT caretpos;
			byte stat;
		};

	private :
		std::stack<BInfo> undo_;		

	public :
		BInfo Undo();
		void AddChar(UINT pos, UINT len, byte stat);
		void Delete(LPCWSTR str, UINT pos0, UINT pos1, byte stat);
		void Clear();
		void UndoAdjust();
};

#define IME_STAT_FIRST 99

class CTextContainer
{
	public:
		CTextContainer(); 
		virtual ~CTextContainer();

		BOOL InsertText(UINT nPos, const WCHAR *psz, UINT nCnt, UINT& nResultCnt, bool undo_process=true);
		BOOL RemoveText(UINT nPos, UINT nCnt, bool undo_process=true);
		UINT GetText(UINT nPos, WCHAR *psz, UINT nBuffSize);
		UINT GetTextLength() const {return nTextSize_;}
		const WCHAR *GetTextBuffer() const {return psz_;}
		void Clear();
		void Reset();

		int SelStart() const { return nSelStart_; }
		int SelEnd() const { return nSelEnd_; }
		bool SelTrail() const { return bSelTrail_; }

		void SetSelStart(int a){ nSelStart_ = a; }
		void SetSelEnd(int a){ nSelEnd_ = a; }


		LONG AddTab(int row, bool bAdd );
		int  LineNo(LONG nPos) const;

		UndoTextEditor::BInfo Undo();
		void UndoAdjust();

		std::wstring GetRowText(int pos);

		bool IsShowHScrollbar() const;
		bool IsShowVScrollbar() const;

	private :
		int nSelStart_, nSelEnd_;
	public :
		bool bSelTrail_;
		CRect rc_;
		SIZE view_size_;
		bool bSingleLine_;
	
		CPoint offpt_;
		int nStartCharPos_;

		byte ime_stat_;
		int top_row_idx_;
		int scrollbar_offx_;
		float line_width_max_;
		float line_height_max_;
		CRect vscbar_rc_, hscbar_rc_;
	private:
		BOOL EnsureBuffer(UINT nNewTextSize);
		const UINT LimitCharCnt_ = 65000;
		WCHAR* psz_;
		UINT nBufferCharCount_;
		UINT nTextSize_;
		std::shared_ptr<UndoTextEditor> undo_;

};
};
