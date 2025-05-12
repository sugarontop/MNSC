#pragma once

#include <memory>
#include <vector>
#include <map>

#define VTF_FUNC			(VT_RESERVED+1)
#define VTF_BUILTIN_FUNC	(VT_RESERVED+2)
#define VT_CONST	(0x800)
#define VTCT(vt)	((vt)&~VT_CONST)

enum FVariantType {ARRAY=1,MAP=2, FUNCTIONPOINTER =3, CLASS=4};

class const_struct
{
	public:
		const_struct() :lock_(false) {}
		bool lock_;
};

class FVariant : private tagVARIANT, private const_struct
{	
	public :
		using tagVARIANT::vt;
		using tagVARIANT::bstrVal;
		using tagVARIANT::intVal;
		using tagVARIANT::iVal;
		using tagVARIANT::punkVal;
		using tagVARIANT::dblVal;
		using tagVARIANT::llVal;
		using tagVARIANT::boolVal;
		using tagVARIANT::fltVal;

	public:
		FVariant() { init(); }
		FVariant(const FVariant& var);
		FVariant(VARIANT v);
		
		explicit FVariant(LPCWSTR str);
		explicit FVariant(__int64 x);
		explicit FVariant(double x);
		explicit FVariant(int x);
		explicit FVariant(bool x);
		
		~FVariant() { clear(); }

		void InnerCopy(const VARIANT* src);
	public:
		void init();
		void clear();
		void detach();
		inline void clear_init();
		void setN(__int64 value) {  vt = VT_I8; llVal = value; }
		void setD(double value) {  vt = VT_R8; dblVal = value; }
		void setS(const TCHAR* str) {  vt = VT_BSTR; bstrVal = ::SysAllocString(str); }
		void setS(const std::wstring& str) {  vt = VT_BSTR; bstrVal = ::SysAllocString(str.c_str()); }
		void setBSTR(BSTR str) {  vt = VT_BSTR; bstrVal = ::SysAllocString(str); }
		void setBL(bool x) { vt = VT_BOOL; boolVal = (x ? -1 : 0);}
		void setAr(const std::vector<FVariant>& ar);
		void set(const FVariant& var);
		void setAr(const FVariant& key, const FVariant& var);
		FVariant getAr(int idx) const;
		void setMap(std::map<wstring, FVariant>& map);
		void setUnknown(const FVariant& var);
		void setDic(const FVariant& key, const FVariant& val);
		bool getDic(const FVariant& key, FVariant* ret) const;
	
		VARTYPE VT() const { return this->vt; }
		__int64	getN() const;
		double	getD() const;
		const BSTR	getS() const { return bstrVal; }
		bool getBL() const ;
		VARIANT ToVARIANT() const;
		void ToCopy(VARIANT* dst) const;
		void FromCopy(const VARIANT* src);
		const BSTR getSS() const { return bstrVal; }
		ULONG	length() const;
		void	toStr();
		void lock(){ lock_ = true; }
		void unlock() { lock_ = false; }
		
	public:
		const FVariant& operator = (const FVariant& v);
		const FVariant& operator = (const VARIANT& v);
	
		

		FVariant& operator += (const FVariant& v);
		FVariant& operator += (const LPCTSTR str);
		friend FVariant operator + (const FVariant& var1, const FVariant& var2);
		friend FVariant operator - (const FVariant& var1, const FVariant& var2);
		friend FVariant operator * (const FVariant& var1, const FVariant& var2);
		friend FVariant operator / (const FVariant& var1, const FVariant& var2);
		friend FVariant operator % (const FVariant& var1, const FVariant& var2);

		friend FVariant operator || (const FVariant& var1, const FVariant& var2);
		friend FVariant operator && (const FVariant& var1, const FVariant& var2);

		BOOL operator > (const FVariant&) const;
		BOOL operator < (const FVariant&) const;
		BOOL operator == (const FVariant&) const;
		BOOL operator != (const FVariant&) const;
		BOOL operator >= (const FVariant&) const;
		BOOL operator <= (const FVariant&) const;

};