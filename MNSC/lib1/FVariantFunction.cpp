#include "pch.h"
#include "FVariant.h"
#include "FVariantFunction.h"
#include "FVariantArray.h"
#include <regex>
// VARIANT VarInvoke(wstring funcnm, bool* bUpdate, VARIANT* v, int vcnt)


#pragma region STATICFUNC

static VARIANT length(const VARIANT v, const VARIANT, const VARIANT, const VARIANT)
{
	if ( v.vt == VT_BSTR )
	{
		auto len = ::SysStringLen(v.bstrVal);
		FVariant v((long long)len);
		return v.ToVARIANT();
	}
	return FVariant(0).ToVARIANT(); 
}
static VARIANT toUpper(const VARIANT v, const VARIANT, const VARIANT, const VARIANT)
{
	if (v.vt == VT_BSTR)
	{
		BSTR s = ::SysAllocString(v.bstrVal);
		auto len = ::SysStringLen(s);

		const int off = 'A' - 'a';
		for (UINT i = 0; i < len; i++)
		{
			auto& ch = s[i];

			if ('a' <= ch && ch <= 'z')
				ch = ch + off;
		}
		
		return FVariant(s).ToVARIANT();
	}
	return FVariant(L"").ToVARIANT();
}
static VARIANT toLower(const VARIANT v, const VARIANT, const VARIANT, const VARIANT)
{
	if (v.vt == VT_BSTR)
	{
		BSTR s = ::SysAllocString(v.bstrVal);
		auto len = ::SysStringLen(s);

		const int off = 'A' - 'a';
		for (UINT i = 0; i < len; i++)
		{
			auto& ch = s[i];

			if ('a' <= ch && ch <= 'z')
				ch = ch - off;
		}

		return FVariant(s).ToVARIANT();
	}
	return FVariant(L"").ToVARIANT();
}


static VARIANT substr(const VARIANT target, const VARIANT spos, const VARIANT len, const VARIANT)
{
	if (target.vt == VT_BSTR)
	{		
		BSTR s = target.bstrVal;
		UINT ispos = (UINT)spos.llVal;
		UINT iepos = min(::SysStringLen(s), ((UINT)len.llVal + ispos));

		if ( ispos < iepos )
		{
			auto len = iepos-ispos;

			std::vector<WCHAR> cb(len+1);
			memcpy( &cb[0], s+ispos, sizeof(WCHAR)*len);
			cb[len]=0;
					
			FVariant ret;
			ret.setBSTR(&cb[0]);

			return ret.ToVARIANT();		
		}
		else if (ispos == iepos )
		{
			FVariant ret;
			ret.setS(L"");
			return ret.ToVARIANT();
		}
		else
			THROW(L"substr err");
	}
	return FVariant(L"").ToVARIANT();
}
static VARIANT toNumber(const VARIANT target, const VARIANT , const VARIANT , const VARIANT)
{
	if (target.vt == VT_BSTR)
	{
		BSTR s = target.bstrVal;
		FVariant ret(_wtoi64(s));
		return ret.ToVARIANT();
	}
	return FVariant((__int64)0).ToVARIANT();
}
static VARIANT toLeft(const VARIANT target, const VARIANT len, const VARIANT empty, const VARIANT)
{
	return substr(target, FVariant((long long)0).ToVARIANT(), len, empty);
}
static VARIANT toStr(const VARIANT target, const VARIANT , const VARIANT , const VARIANT)
{
	FVariant ret = target;
	ret.toStr();
	//return ret;	

	VARIANT vret;
	::VariantInit(&vret);
	ret.ToCopy(&vret);
	return vret;


}
static VARIANT getSquare(const VARIANT target, const VARIANT pos, const VARIANT empty, const VARIANT)
{
	FVariant ret;
	if (target.vt == VT_BSTR)
	{
		ret = substr(target,pos,FVariant(1).ToVARIANT(), empty);
	}
	return ret.ToVARIANT();
}
static VARIANT Split(const VARIANT target, const VARIANT split_str, const VARIANT, const VARIANT)
{
	FVariant ret;
	if (target.vt == VT_BSTR && split_str.vt == VT_BSTR)
	{
		auto ar = new IVARIANTArrayImp();
		
		std::wstring str = target.bstrVal;
		std::wregex re( split_str.bstrVal );
		std::wsregex_token_iterator it(str.begin(), str.end(), re, -1);
		std::wsregex_token_iterator end;

		while (it != end) {
			auto& item = *it++;
			FVariant x(item.str().c_str());
			VARIANT vx = x.ToVARIANT();
			ar->Add(vx);
		}

		ret.vt = VT_UNKNOWN;
		ret.punkVal = ar;
	}
	return ret.ToVARIANT();
}
static VARIANT strReplace(const VARIANT target, const VARIANT oldstr, const VARIANT newstr, const VARIANT)
{
	FVariant ret;
	if (target.vt == VT_BSTR && oldstr.vt == VT_BSTR && newstr.vt == VT_BSTR)
	{		
		if ( ::SysStringLen(oldstr.bstrVal) == 1 && ::SysStringLen(newstr.bstrVal) == 1 )
		{
			wstring str = target.bstrVal;
			WCHAR chold, chnew;
			chold = oldstr.bstrVal[0];
			chnew = newstr.bstrVal[0];
			std::replace(str.begin(), str.end(), chold, chnew);

			return FVariant(str.c_str()).ToVARIANT();
		}
		else if (::SysStringLen(oldstr.bstrVal) >= 1 && ::SysStringLen(newstr.bstrVal) >= 1)
		{
			auto replaceAll = [](std::wstring & str,
				const std::wstring & oldStr,
				const std::wstring & newStr) {
				size_t pos = 0;
				while ((pos = str.find(oldStr, pos)) != std::wstring::npos) {
					str.replace(pos, oldStr.length(), newStr);
					pos += newStr.length(); // íuä∑å„ÇÃï∂éöóÒÇÃå„ÇÎÇ©ÇÁåüçıÇåpë±
				}
			};

			wstring str = target.bstrVal;
			wstring olds = oldstr.bstrVal;
			wstring news = newstr.bstrVal;

			replaceAll(str,olds,news);

			return FVariant(str.c_str()).ToVARIANT();

		}
	}

	return target;
}


#pragma endregion

void VarFunctionInit(std::map<wstring, VARIANTFUNC>& map)
{
	map[L"length"] = length;
	map[L"toUpper"] = toUpper;
	map[L"toLower"] = toLower;
	map[L"substr"] = substr;
	map[L"toNumber"] = toNumber;
	map[L"left"] = toLeft;
	map[L"str"] = toStr;
	map[L"split"] = Split;
	map[L"[]"] = getSquare;
	map[L"replace"] = strReplace;
}