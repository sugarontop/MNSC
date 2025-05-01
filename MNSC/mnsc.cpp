#include "pch.h"
#include "mnsc.h"
#include "FScript.h"
#include "lib1\FScriptEasy.h"
#include "lib1\FVariantArray.h" 

inline FScript* Handle(ScriptSt st)
{
	return (FScript*)st.p;
}

std::wstring _A2W(std::string& s);

DLLEXPORT ScriptSt MNSCInitilize(LPVOID sender, DWORD script_engine_type)
{
	ScriptSt st = {};
	st.sender = sender;

	if (script_engine_type == 1)
	{
		FScript* sc = new FScriptEasy();
		st.p = sc;
		st.result = true;
	}
	return st;
}
DLLEXPORT bool MNSCParse(ScriptSt& st, LPCWSTR script, LPCWSTR appnm,const VARIANT app)
{
	st.result = false;
	try
	{
		auto sc = Handle(st);
		st.result = sc->Parse(script, appnm, app);
	}
	catch(std::runtime_error& er)
	{
		std::string s = er.what();
		st.error_msg = ::SysAllocString( _A2W(s).c_str());
	}
	catch(...)
	{
		st.error_msg = ::SysAllocString(L"throw catched");
	}
	return st.result;

}

DLLEXPORT VARIANT MNSCCall(ScriptSt& st, LPCWSTR funcnm, VARIANT* prms, int pmcnt)
{
	_ASSERT(st.result == true);
	VARIANT ret;
	::VariantInit(&ret);
	
	try
	{
		auto sc = Handle(st);
	
		ret = sc->Call(funcnm, prms, pmcnt);

		st.result = true;
	}
	catch (std::runtime_error& er)
	{
		std::string s = er.what();
		st.error_msg = ::SysAllocString(_A2W(s).c_str());	
		st.result = false;
	}
	catch (...)
	{
		st.error_msg = ::SysAllocString(L"runtime_error, throw catched");		
		st.result = false;
	}

	return ret;
}

DLLEXPORT void MNSCClose(ScriptSt& st)
{
	if (st.p)
		delete Handle(st);
	if (st.error_msg)
		::SysFreeString(st.error_msg);
	
	st.p = nullptr;
	st.error_msg = nullptr;
}

DLLEXPORT VARIANT MNSCCreateMap()
{
	VARIANT ret;
	::VariantInit(&ret);
	ret.punkVal = new IVARIANTMapImp();
	ret.vt = VT_UNKNOWN;
	return ret;
}

DLLEXPORT VARIANT MNSCCreateArray()
{
	VARIANT ret;
	::VariantInit(&ret);
	ret.punkVal = new IVARIANTArrayImp();
	ret.vt = VT_UNKNOWN;
	return ret;
}



DLLEXPORT bool MNSCReadUtf8(LPCWSTR fnm, BSTR* ret)
{
	FILE* cf=nullptr;
	if ( 0 ==_wfopen_s(&cf, fnm, L"rb") && cf!=0)
	{
		char cb[256] = {};
		std::stringstream sm;

		unsigned char bom[3] = {};
		size_t bytesRead = fread(bom, 1, 3, cf);
		if (bytesRead == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
		{
			// UTF-8 BOMÅi0xEF, 0xBB, 0xBFÅjÇîªíË				
		}
		else
		{
			sm.write(reinterpret_cast<char*>(bom), bytesRead);
		}

		
		while ((bytesRead = fread_s(cb, sizeof(cb), sizeof(char), sizeof(cb), cf)) > 0)
		{
			sm.write(cb, bytesRead);
		}
		fclose(cf);

		auto cstr = sm.str();
		int len = ::MultiByteToWideChar(CP_UTF8, 0, cstr.c_str(), -1, NULL, NULL);
		std::vector<WCHAR> ar(len);
		if (0!=::MultiByteToWideChar(CP_UTF8, 0, cstr.c_str(), -1, ar.data(), len))
			*ret = ::SysAllocString(ar.data());

		return true;
	}
	return false;
}


DLLEXPORT bool MNSCWriteUtf8(LPCWSTR fnm, BSTR text)
{
	FILE* cf = nullptr;
	if (0 == _wfopen_s(&cf, fnm, L"wb") && cf != 0 && text)
	{
		int len = ::SysStringLen(text);
		int ulen = ::WideCharToMultiByte(CP_UTF8,0,text,len,nullptr,0,nullptr,nullptr);
		std::vector<char> ar(ulen);
		if (0 != ::WideCharToMultiByte(CP_UTF8, 0, text, len, ar.data(), ulen, nullptr,nullptr))
		{			
			fwrite(ar.data(),1,ulen,cf);
		}
		fclose(cf);
		return true;
	}
	return false;
}

std::wstring _A2W(std::string& s)
{
	int wlen = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.length(), nullptr, 0);
	std::unique_ptr<WCHAR[]> wcb(new WCHAR[wlen]);
	::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.length(), wcb.get(), wlen);
	return std::wstring(wcb.get(), wlen);
}