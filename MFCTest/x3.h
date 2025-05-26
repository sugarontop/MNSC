#pragma once
#include "x.h"

class IVARIANTTextbox : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTTextbox(const CRect& rc) :rc_(rc), caret_xpos_(0)
	{
	}

private:
	CRect rc_;
	std::wstring text_;
	HWND hWnd_;
	int caret_xpos_;
	std::vector<CRect> char_rects_;
public:
	virtual void Clear() {}
	virtual int TypeId() { return 2002; }

	std::vector<std::wstring> items;

	VARIANT setText(VARIANT  txt)
	{

		if (txt.vt == VT_BSTR)
		{
			text_ = txt.bstrVal;

			
		}


		CComVariant ret(0);
		return ret;
	}
	
	
public :
	virtual void Draw(CDC* pDC)
	{
		if (char_rects_.empty())
		{
			CalcCharRect(pDC);
			MoveCaret(0);
		}

		pDC->FillSolidRect(rc_,RGB(255,255,255));

		pDC->DrawTextExW(
				const_cast<LPWSTR>(text_.c_str()),
				static_cast<int>(text_.length()),
				&rc_,
				DT_VCENTER | DT_SINGLELINE,
				nullptr
			);
	}
	virtual void setText(const std::wstring& txt) 
	{ 
		text_ = txt; 
		char_rects_.clear();
	}

	virtual void setRect(const CRect& rc) { rc_ = rc; }
	virtual CRect getRect() { return rc_; }

	void SetFocus(HWND hWnd, CPoint pt)
	{
		hWnd_ = hWnd;
		CreateCaret(hWnd,NULL,2,rc_.Height());

		SetCaret(pt);
	}
	void SetCaret(CPoint pt)
	{
		pt.x -= rc_.left;

		int i = (int)char_rects_.size();
		int off=0;
		
		if (char_rects_[i-1].right < pt.x)
		{
			int new_xpos = i+1;
			off = new_xpos - caret_xpos_;			
		}
		else
		{
			for(i = 0; i < (int)char_rects_.size(); i++)
			{
				if (char_rects_[i].left < pt.x && pt.x < char_rects_[i].right )
				{
					int new_xpos = i;
					off = new_xpos - caret_xpos_;
					break;
				}
			}
		}

		MoveCaret(off);
		::ShowCaret(hWnd_);
	}
	void ReleaseFocus()
	{
		::HideCaret(hWnd_);


	}

	void AddChar(WCHAR ch)
	{
		if ( ch > 127 || ch < ' ' ) return;

		std::wstring a = text_.substr(0, caret_xpos_);
		std::wstring b = text_.substr(caret_xpos_, text_.length()-caret_xpos_);

		a += ch;
		a += b;

		text_ = a;
		caret_xpos_++;
		char_rects_.clear();
	}
	void CalcCharRect(CDC* pDC)
	{
		CRect rc(0,0,0,rc_.Height());
		int i = 0;

		char_rects_.clear();
		char_rects_.resize(text_.length());

		for(WCHAR ch : text_)
		{
			int width;
			pDC->GetCharWidth(ch,ch,&width);
			rc.right += width;
			char_rects_[i++]=rc;
			rc.left += width;
		}
	}

	void MoveCaret(int offx)
	{
		caret_xpos_ = max(0,min((int)text_.length(), caret_xpos_+offx));

		::HideCaret(hWnd_);
		if (caret_xpos_ == 0)
			SetCaretPos(rc_.left, rc_.top);
		else if (caret_xpos_ == text_.length())
			SetCaretPos(rc_.left + char_rects_[caret_xpos_-1].right, rc_.top);
		else
			SetCaretPos(rc_.left+ char_rects_[caret_xpos_].left, rc_.top);
		::ShowCaret(hWnd_);
	}
	void MoveCaret2(bool last)
	{
		::HideCaret(hWnd_);
		if (last)
		{
			caret_xpos_ = text_.length();
			SetCaretPos(rc_.left + char_rects_[caret_xpos_ - 1].right, rc_.top);
		}
		else
		{
			caret_xpos_ = 0;
			SetCaretPos(rc_.left, rc_.top);
		}
		::ShowCaret(hWnd_);
	}
	void DeleteChar(bool bDeleteKey)
	{
		if ( bDeleteKey )
		{
			if (caret_xpos_  < text_.length())
			{
				std::wstring a = text_.substr(0, caret_xpos_);
				std::wstring b = text_.substr(caret_xpos_+1, text_.length() - (caret_xpos_+1));

				a += b;
				text_ = a;
			
				char_rects_.clear();
			}
		}
		else
		{	
			if ( 0 < caret_xpos_)
			{
				std::wstring a = text_.substr(0, caret_xpos_-1);
				std::wstring b = text_.substr(caret_xpos_, text_.length()-caret_xpos_ );
				a += b;
				text_ = a;
				caret_xpos_ = max(0, caret_xpos_-1);
				char_rects_.clear();
			}
		}


	}


public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
		if (riid == IID_IUnknown) {
			*ppv = static_cast<IUnknown*>(this);
		}
		else {
			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	virtual VARIANT Invoke(LPCWSTR cfuncnm, VARIANT* v, int vcnt) override
	{
		std::wstring funcnm = cfuncnm;
		if (funcnm == L"settext" && vcnt > 0)
		{
			return setText(v[0]);
		}

		throw(std::wstring(L"Invoke err"));

	}
}; 
