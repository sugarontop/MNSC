#pragma once
#include "FVariant.h"

class CPrsSymbol;

class CSymbolTable 
{
public:
	CSymbolTable();
	CSymbolTable(CPrsSymbol* parse);
	CSymbolTable(const CSymbolTable* parent);
	CSymbolTable(const CSymbolTable& src);
	~CSymbolTable() ;
private:
	const CSymbolTable* m_parent;
	CPrsSymbol* m_parse;
	std::map<wstring, FVariant> map_;
	
public:
	enum type { TYPE_LONG, TYPE_DOUBLE, TYPE_STR, TYPE_FUNC, TYPE_OBJECT, TYPE_EXPORT, TYPE_EXPORTDATA, TYPE_BUILTIN_FUNC};
	bool  findTable(const wstring& Ident, int& type);

	void set(const wstring& Ident, int type);
	void setAt(const wstring& Ident, FVariant& var);
	int  getAt(const wstring& Ident, FVariant& var);
	int  updateAt(const wstring& Ident, FVariant& var);
	bool isContain(const wstring& Ident);
	void removeAt(const wstring& ident);

	
	void clear();
	void clear_exp();
	
};