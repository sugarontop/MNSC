#pragma once
#include "x.h"

class IVARIANTTextbox : public IVARIANTAbstract, public DrawingObject
{
public:
	IVARIANTTextbox(const CRect& rc) :rc_(rc), caret_xpos_(0), border_(0), readonly_(FALSE), multiline_(FALSE), line_height_(0)
	{
	}

private:
	CRect rc_;
	std::wstring text_;
	HWND hWnd_;
	int caret_xpos_;
	std::vector<CRect> char_rects_;
	int border_;
	bool readonly_;
	bool multiline_;
	int line_height_;
public:
	virtual void Clear() {}
	virtual int TypeId()  const { return 2002; }

	VARIANT setText(VARIANT  txt)
	{

		if (txt.vt == VT_BSTR)
		{
			text_ = txt.bstrVal;
			char_rects_.clear();
			
		}


		_variant_t ret(0);
		return ret;
	}
	VARIANT setProperty(VARIANT border, VARIANT readonly, VARIANT multiline)
	{
		if (border.vt == VT_I8 || border.vt == VT_INT)
			border_ = border.intVal;

		if (readonly.vt == VT_BOOL)
			readonly_ = readonly.boolVal;

		if (multiline.vt == VT_BOOL)
			multiline_ = multiline.boolVal;

		_variant_t ret(0);
		return ret;
	}
	
public :

	

	virtual void Draw(CDC* pDC)
	{
		if (char_rects_.empty())
		{
			TEXTMETRIC tm;
			pDC->GetTextMetrics(&tm);
			line_height_ = tm.tmHeight + tm.tmExternalLeading;

			CalcCharRect(pDC);
			MoveCaret(0);
		}
		
		
			
		pDC->FillSolidRect(rc_,(readonly_ ? RGB(210,210,210): RGB(255, 255, 255)));

		pDC->DrawTextExW(
				const_cast<LPWSTR>(text_.c_str()),
				static_cast<int>(text_.length()),
				&rc_,
				DT_TOP | DT_LEFT,  //DT_VCENTER | DT_SINGLELINE,
				nullptr
			);


		if (border_)
		{
			CBrush br;
			br.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));
			pDC->FrameRect(rc_, &br);
		}
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
		CreateCaret(hWnd, NULL, 3, line_height_);

		SetCaret(pt);
		::ShowCaret(hWnd_);
	}
	void SetCaret(CPoint pt)
	{
		pt.x -= rc_.left;
		pt.y -= rc_.top;

		int new_xpos = (int)char_rects_.size();
		for (int j = 0; j < char_rects_.size(); j++)
		{
			CRect rc = char_rects_[j];
			CPoint pt2(pt.x + rc.Width() / 2, pt.y);
			if (rc.PtInRect(pt2))
			{
				new_xpos = j;
				break;
			}
		}

		int off = new_xpos - caret_xpos_;

		MoveCaret(off);
	}
	void ReleaseFocus()
	{
		::HideCaret(hWnd_);


	}

	void AddChar(WCHAR ch)
	{
		if ((ch > 127 || ch < ' ') && ch != '\r' && ch != '\t') return;
		if (!multiline_ && ch == '\r') return;

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
		CRect rc(0, 0, 0, rc_.Height());

		char_rects_.clear();
		char_rects_.resize(text_.length());

		LPCWSTR str = text_.c_str();
		int k = 0;
		for (int i = 0; i < (int)text_.length(); i++)
		{
			CSize sz;
			GetTextExtentPoint32(pDC->GetSafeHdc(), str + k, (i - k) + 1, &sz);

			rc.right = rc.left + (sz.cx - rc.left);
			rc.bottom = rc.top + sz.cy;

			if (str[i] == '\r')
			{
				rc.left = rc.right;
				char_rects_[i] = rc;

				rc.left = rc.right = 0;
				rc.top += (rc.bottom - rc.top);
				k = i + 1;
			}
			else
			{
				char_rects_[i] = rc;
				rc.left = rc.right;
			}
		}
	}

	void MoveCaret(int offx)
	{
		caret_xpos_ = max(0, min((int)text_.length(), caret_xpos_ + offx));

		::HideCaret(hWnd_);
		if (caret_xpos_ == 0)
			SetCaretPos(rc_.left, rc_.top);
		else if (caret_xpos_ == text_.length())
		{
			auto rc = char_rects_[caret_xpos_ - 1];

			if (rc.Width() == 0)
				SetCaretPos(rc_.left, rc_.top + rc.top + rc.Height());
			else
				SetCaretPos(rc_.left + char_rects_[caret_xpos_ - 1].right, rc_.top + char_rects_[caret_xpos_ - 1].top);
		}
		else
			SetCaretPos(rc_.left + char_rects_[caret_xpos_].left, rc_.top + char_rects_[caret_xpos_].top);


		::ShowCaret(hWnd_);
	}
	void MoveCaret2(bool last)
	{
		::HideCaret(hWnd_);
		if (last)
		{
			caret_xpos_ = (int)text_.length();
			SetCaretPos(rc_.left + char_rects_[caret_xpos_ - 1].right, rc_.top + char_rects_[caret_xpos_ - 1].top);
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

	bool ReadOnly() const { return readonly_; }


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
