#pragma once

#ifdef _WINDLL
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C"
#endif

typedef VARIANT(*APPFUNC)(LPVOID sender, const VARIANT a, const VARIANT b, const VARIANT c, const VARIANT d);


struct ScriptSt
{
	LPVOID sender;
	LPVOID p;
};

DLLEXPORT ScriptSt MNSCInitilize(LPVOID sender, DWORD script_engine_type); 

DLLEXPORT bool MNSCParse(ScriptSt st, LPCWSTR script, LPCWSTR appnm, const VARIANT app);

DLLEXPORT VARIANT MNSCCall(ScriptSt st, LPCWSTR funcnm, VARIANT* prm=nullptr, int pmcnt=0);

DLLEXPORT void MNSCClose(ScriptSt st);


DLLEXPORT VARIANT MNSCCreateMap();
DLLEXPORT VARIANT MNSCCreateArray();

class IVARIANTAbstract : public IUnknown
{
public:
	IVARIANTAbstract() :ref(1) {}
	virtual ~IVARIANTAbstract() {}
	virtual ULONG __stdcall AddRef() { ref++; return ref; }
	virtual ULONG __stdcall Release() { ref--; if (ref == 0) { Clear(); delete this; return 0; } return ref; }
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) = 0;
protected:
	ULONG ref;
public:
	
	virtual int TypeId() = 0;
	virtual void Clear() = 0;
	virtual VARIANT Invoke( std::wstring funcnm, VARIANT* v, int vcnt ) = 0;
};


class IVARIANTArray : public IVARIANTAbstract
{
public:
	IVARIANTArray() {}
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) = 0;
	virtual int TypeId() = 0;
	virtual void Clear() = 0;
	virtual VARIANT Invoke(std::wstring funcnm, VARIANT* v, int vcnt) = 0;
public:
	virtual void Add(VARIANT& v) = 0;
	virtual bool Get(int idx, VARIANT*) = 0;
	virtual ULONG Count() = 0;
	virtual void Set(int idx, VARIANT& v) = 0;
	
};
class IVARIANTMap : public IVARIANTAbstract
{
public:
	IVARIANTMap() {};
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) = 0;
	virtual int TypeId() = 0;
	virtual void Clear() = 0;
	virtual VARIANT Invoke(std::wstring funcnm, VARIANT* v, int vcnt) = 0;
public:
	virtual void SetItem(const std::wstring& key, VARIANT& v) = 0;
	virtual bool GetItem(const std::wstring& key, VARIANT* v) = 0;	
};


