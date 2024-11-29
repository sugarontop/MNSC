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