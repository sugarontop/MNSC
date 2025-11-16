#include "pch.h"
#include "TextContainer.h"

using namespace TSF;

//void CTextContainer::UndoAdjust(), temporary stop

#define MINI_BUFFER_SIZE	16

CTextContainer::CTextContainer()
{
	nSelStart_ = nSelEnd_ = 0;
	bSelTrail_ = false;
	bSingleLine_ = false;
	
	nStartCharPos_ = 0;
	psz_ = 0;
	nTextSize_ = 0;
	view_size_ = {0};
	nBufferCharCount_ = 0;
	undo_ = std::make_shared<UndoTextEditor>();
	
	top_row_idx_ = 0;
	scrollbar_offx_ = 0;
	line_width_max_ = 0;
	row_cnt_ = 0;

	EnsureBuffer(MINI_BUFFER_SIZE);
}
CTextContainer::~CTextContainer()
{
	Clear();
}


UndoTextEditor::BInfo CTextContainer::Undo()
{
	return undo_->Undo();
}

BOOL CTextContainer::InsertText(UINT nPos, const WCHAR *psz, UINT nCnt, UINT& nResultCnt, bool undo_process)
{ 	
	WCHAR xx = *psz;
	
	
	
	
	if ( LimitCharCnt_ < GetTextLength() + nCnt )
	{
		nCnt = LimitCharCnt_ - GetTextLength();
	}
	else if ( nBufferCharCount_ == 0 && nCnt == 0 )
	{		
		nBufferCharCount_ = MINI_BUFFER_SIZE;
		psz_ = new WCHAR[nBufferCharCount_];
		memset(psz_, 0, nBufferCharCount_*sizeof(WCHAR) );

		return TRUE;
	}

	if (!EnsureBuffer(nTextSize_ + nCnt))
	{
		return FALSE;
	}

	_ASSERT(nTextSize_ + nCnt < nBufferCharCount_);

	// move target area text to last.
	_ASSERT( 0<=nPos );
	_ASSERT(nPos <= nTextSize_ && -1 != (int)nTextSize_);
	
	memmove(psz_ + nPos + nCnt, psz_ + nPos, (nTextSize_ - (UINT)nPos) * sizeof(WCHAR));
	
	// add new text
	memcpy(psz_ + nPos, psz, nCnt * sizeof(WCHAR));

	nTextSize_ += nCnt;

	psz_[nTextSize_] = 0;

	if ( undo_process )
		undo_->AddChar( nPos, nCnt );

	nResultCnt = nCnt;
	return TRUE;
}

bool CTextContainer::IsShowHScrollbar() const
{
	return (rc_.Width() < line_width_max_);
}
bool CTextContainer::IsShowVScrollbar() const
{
	return (row_cnt_ > 1);
}

std::wstring CTextContainer::GetRowText(int pos)
{
	int a = pos;
	
	while( 0 < a && psz_[a-1] !=L'\n')
		a--;

	int len = pos - a;

	return std::wstring(psz_+a, len);
}

BOOL CTextContainer::RemoveText(UINT nPos, UINT nCnt, bool undo_process)
{ 
	if (!nCnt)
		return TRUE;

	if (nPos + nCnt - 1 > nTextSize_)
		nCnt = nTextSize_ - nPos;


	auto start =  psz_ + nPos;
	auto end =  psz_ + nPos + nCnt;

	if (undo_process)
		undo_->Delete(psz_, nPos, nPos+nCnt);


	int cnt = (int)nTextSize_-(nPos+nCnt);

	if (cnt > 0 )
	{
		memmove( start, end,  sizeof(WCHAR)*cnt );
	}

	nTextSize_ -= nCnt;
	psz_[nTextSize_] = 0;

	return TRUE;
}

UINT CTextContainer::GetText(UINT nPos, WCHAR *psz, UINT nCnt)
{ 
	if (!nCnt)
		return 0;

	auto start =  psz_ + nPos;

	if (nPos + nCnt - 1 > nTextSize_)
		nCnt = nTextSize_ - nPos;

	memcpy(psz, start, sizeof(WCHAR)*nCnt);

	return nCnt;
}

bool CTextContainer::GetSelectionText(std::wstring* ret)
{
	int s1 = SelStart();
	int e1 = SelEnd();

	if ( s1 < e1 )
	{
		int cblen = e1 - s1;
		WCHAR* cb = new WCHAR[cblen + 1];
		GetText((UINT)s1, cb, cblen);
		cb[cblen] = 0;
		*ret = cb;
		delete cb;
		return true;
	}
	return false;
}

void CTextContainer::Clear()
{ 
	delete [] psz_;
	psz_ = nullptr;
	nBufferCharCount_ = 0;
	nTextSize_ = 0;

	vscbar_rc_ = {};
	hscbar_rc_ = {};

	
	top_row_idx_ = 0;
	scrollbar_offx_ = 0;
	line_width_max_ = 0;
	row_cnt_ = 0;


	nSelStart_= nSelEnd_=0;
	bSelTrail_ = false;
	offpt_={};
	nStartCharPos_ = 0;
	undo_->Clear();
}
void CTextContainer::UndoAdjust()
{
//	undo_->UndoAdjust();

}
void CTextContainer::Reset()
{
	Clear();
	EnsureBuffer(MINI_BUFFER_SIZE);
}

BOOL CTextContainer::EnsureBuffer(UINT nNewTextSize)
{ 
	if ( nNewTextSize != MINI_BUFFER_SIZE )
		nNewTextSize = (max(256, nNewTextSize) + 255)/256*256;
	
	if ( nBufferCharCount_ < nNewTextSize )
	{
		auto psz2 = new WCHAR[nNewTextSize+1];
		if (psz2 == nullptr)
			return FALSE;
	
		memset(psz2,0, nNewTextSize + 1);

		if(psz_)
			memcpy(psz2,psz_,nBufferCharCount_*sizeof(WCHAR));

		delete [] psz_;
		psz_ = psz2;
		nBufferCharCount_ = nNewTextSize+1;

		return TRUE;
	}
	return TRUE;
}

LONG CTextContainer::AddTab(int row, bool bAdd )
{
	UINT nResultCnt;
	int irow = 0, pos = 0, head_tabcnt=0, head=0;

	auto ch = psz_[pos];
	while(ch!=0)
	{
		if (row == irow)
			break;
		
		ch = psz_[pos];
		if (ch == L'\n')
		{
			irow++;
			head_tabcnt = 0;
			head=0;
		}
		else if ( ch == L'\t' && head == 0)
			head_tabcnt++;
		else
			head++;

		pos++;
	}

	if ( bAdd )
	{
		//head_tabcnt = max(1,head_tabcnt);

		for(int i=0; i < head_tabcnt; i++ )
			InsertText(pos, L"\t",1, nResultCnt);
	}
	else
		RemoveText(pos, 1);

	while( psz_[pos] != L'\n' && psz_[pos] != L'\0' )
	{
		pos++;
	}

	return pos;
}
int CTextContainer::LineNo(LONG nPos) const
{
	int ret = 0;
	for(int i=0; i < nPos; i++)
	{
		if (psz_[i] == L'\n')
			ret++;
	}
	return ret;
}


/////////////////////////////////////////////////////////////

void UndoTextEditor::AddChar(UINT pos,UINT len)
{	
	BInfo b;
	b.caretpos = pos;
	b.len = len;
	b.p = nullptr;
	


	undo_.push(b);

}

void UndoTextEditor::UndoAdjust()
{
	// IME ONして入力開始のstat=10までさかのぼってundo情報を消す。
	auto& undo2 = undo_;

	std::vector<BInfo> ar;

	while( !undo2.empty() )
	{
		auto b = undo2.top();
		
		ar.push_back(b);

		_ASSERT( ar[0].stat == 20 );

		if ( b.stat == 10 )
		{
			undo2.pop();

			undo2.push( ar[0] );
			break;
		}
		undo2.pop();
		
	}

}
UndoTextEditor::BInfo UndoTextEditor::Undo()
{
	BInfo b;	

	while( !undo_.empty() )
	{
		b = undo_.top();

		if ( b.stat != 2 )
		{
			undo_.pop();
			break;
		}

		undo_.pop();
	}

	return b;
}
void UndoTextEditor::Delete(LPCWSTR str, UINT pos0, UINT pos1)
{
	_ASSERT( pos0 <= pos1 );
	
	if ( pos0 == pos1) return;
	
	BInfo b;

	WCHAR* cb = new WCHAR[pos1-pos0+1];
	memcpy(cb, str+pos0, (pos1-pos0)*sizeof(WCHAR));
	cb[pos1-pos0] = 0;

	b.p = std::shared_ptr<WCHAR[]>(cb);
	b.len = (int)pos1- (int)pos0;
	b.caretpos = pos0;
	

	undo_.push(b);
}

void UndoTextEditor::Clear()
{
	std::stack<BInfo> empty;

	undo_ = empty;

}