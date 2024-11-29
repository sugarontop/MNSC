#pragma once
#include "mnsc.h"
#include "FScript.h"

class CPrsProgram;
class CPrsSymbol;
class FScriptEasy : public FScript
{
	public :
		FScriptEasy(){};
		virtual ~FScriptEasy(){ Close();}
		virtual bool Parse(LPCWSTR script, LPCWSTR appnm, VARIANT app) override;
		virtual VARIANT Call(LPCWSTR funcnm, VARIANT* prm, int prmcnt) override;
		virtual void RegistExportFunc(LPCWSTR funcnm, APPFUNC func) override;
		virtual void Close() override;
		virtual APPFUNC GetFunc(LPCWSTR funcnm) override;
	protected :
		std::map<std::wstring, APPFUNC> funcmap_;

		std::shared_ptr <CPrsSymbol> symbol_;
		std::shared_ptr<CPrsProgram> prg_;
};