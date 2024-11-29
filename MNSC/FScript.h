#pragma once
#include "mnsc.h"

class FScript
{
	public :
		FScript(){}
		virtual ~FScript(){}
		virtual bool Parse(LPCWSTR script, LPCWSTR appnm, VARIANT app) =0;
		virtual VARIANT Call(LPCWSTR funcnm, VARIANT* prm, int prmcnt)=0;
		virtual void RegistExportFunc(LPCWSTR funcnm, APPFUNC func)=0;
		virtual void Close()=0;
		virtual APPFUNC GetFunc(LPCWSTR funcnm)=0;
	protected :
		//std::map<CComBSTR, APPFUNC> funcmap_;
};