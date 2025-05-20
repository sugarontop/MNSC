#include "pch.h"
#include "CSymbolTable.h"
///////////////////////////////////////////////////////////////////////////////////
CSymbolTable::CSymbolTable()
{
	m_parent = nullptr;
	m_parse = nullptr;
}
CSymbolTable::CSymbolTable(const CSymbolTable* parent)
{
	_ASSERT(parent);
	m_parent = parent;
	m_parse = parent->m_parse;
	
}
CSymbolTable::CSymbolTable(class CPrsSymbol* parse)
{
	m_parent = nullptr;
	m_parse = parse;
}
CSymbolTable::CSymbolTable(const CSymbolTable& src)
{
	m_parent = src.m_parent;
	m_parse = src.m_parse;
	map_ = src.map_;
	
}
CSymbolTable::~CSymbolTable()
{ 	
	clear();
}
void CSymbolTable::clear()
{
	map_.clear();
}
void CSymbolTable::clear_exp()
{

}



bool CSymbolTable::findTable(const wstring& ident, int& type)
{
	auto it = map_.find(ident);
	if (it != map_.end())
	{
		const FVariant& var = (it->second);
		switch (var.VT())
		{
			case VT_I8: type = TYPE_LONG; break;
			case VT_R8: type = TYPE_DOUBLE; break;
			case VT_BSTR: type = TYPE_STR; break;
			case VTF_FUNC: type = TYPE_FUNC; break;
			case VTF_BUILTIN_FUNC: type = TYPE_BUILTIN_FUNC; break;
		}
		return true;
	}
	return false;
}
void CSymbolTable::removeAt(const wstring& ident)
{
	if (map_.find(ident) == map_.end())
		return;

	map_[ident].clear();
}
void CSymbolTable::setAt(const wstring& ident, FVariant& var)
{
	map_[ident] = var;
}
int CSymbolTable::updateAt(const wstring& ident, FVariant& var)
{
	if (map_.find(ident) == map_.end())
		return -1;

	setAt(ident, var);
	return 0;
}
bool CSymbolTable::isContain(const wstring& ident)
{
	return (map_.find(ident) != map_.end());
}
int  CSymbolTable::getAt(const wstring& ident, FVariant& var)// const
{
	if (map_.find(ident) == map_.end())
		return -1;

	var = map_[ident];

	return 0; // success
}
void CSymbolTable::set(const wstring& key, int type)
{
	if (map_.find(key) != map_.end())
		return;
	
	FVariant nv;
		
	switch (type)
	{
		case TYPE_LONG:
			nv.vt = VT_I8;
			break;
		case TYPE_DOUBLE:
			nv.vt = VT_R8;
			break;
			break;
		case TYPE_STR:
			nv.vt = VT_BSTR;
			break;
		case TYPE_FUNC:
			nv.vt = VTF_FUNC;
			break;
		case TYPE_BUILTIN_FUNC:
			nv.vt = VTF_BUILTIN_FUNC;
		break;
	}
	map_[key] = nv;
}