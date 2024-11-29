#include "pch.h"
#include "FScriptEasy.h"
#include "lib1\CPrsNode.h"

void FScriptEasy::RegistExportFunc(LPCWSTR funcnm, APPFUNC func)
{
	funcmap_[funcnm] = func;
}
void FScriptEasy::Close()
{
	symbol_= nullptr;
	prg_ = nullptr;
}
APPFUNC FScriptEasy::GetFunc(LPCWSTR funcnm)
{
	return funcmap_[funcnm];

}
bool FScriptEasy::Parse(LPCWSTR script, LPCWSTR appnn, VARIANT app)
{
	symbol_ = std::make_shared<CPrsSymbol>(script);
	prg_ = std::make_shared<CPrsProgram>(*symbol_.get());

	try
	{
		prg_->SetPreGlobalVariant(appnn,app);

		prg_->Parse();
		return true;
	}
	catch(wstring err)
	{
		int line = symbol_->getLineNumber();
		std::wstringstream sm;
		sm << L"line number: " << line << L" " << err;
		throw sm.str();
	}
	return false;
}
VARIANT FScriptEasy::Call(LPCWSTR funcnm, VARIANT* prm, int prmcnt)
{
	try
	{
		stackGntInfo info;
		
		prg_->Generate(info,funcnm, prm, prmcnt);

		return prg_->Return().ToVARIANT();
	}
	catch (wstring err)
	{
		throw err;
	}

}