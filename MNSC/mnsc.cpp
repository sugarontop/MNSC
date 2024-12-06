#include "pch.h"
#include "mnsc.h"
#include "FScript.h"
#include "lib1\FScriptEasy.h"
#include "lib1\FVariantArray.h"

inline FScript* Handle(ScriptSt st)
{
	return (FScript*)st.p;
}

DLLEXPORT ScriptSt MNSCInitilize(LPVOID sender, DWORD script_engine_type)
{
	ScriptSt st = {};
	st.sender = sender;

	if (script_engine_type == 1)
	{
		FScript* sc = new FScriptEasy();
		st.p = sc;
	}
	return st;
}
DLLEXPORT bool MNSCParse(ScriptSt st, LPCWSTR script, LPCWSTR appnm,const VARIANT app)
{
	auto sc = Handle(st);
	bool bl = sc->Parse(script, appnm, app);

	return bl;

}

DLLEXPORT VARIANT MNSCCall(ScriptSt st, LPCWSTR funcnm, VARIANT* prms, int pmcnt)
{
	auto sc = Handle(st);
	
	VARIANT ret = sc->Call(funcnm, prms, pmcnt);

	return ret;
}

DLLEXPORT void MNSCClose(ScriptSt st)
{
	if (st.p)
		delete Handle(st);
	
	st.p = nullptr;
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

		size_t bytesRead;
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