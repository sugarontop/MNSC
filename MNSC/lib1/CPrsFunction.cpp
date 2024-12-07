#include "pch.h"
#include "CPrsNode.h"

///////////////////////////////////////////////////////////////////////////////////

CPrsFunction::CPrsFunction(CPrsSymbol& sym, CSymbolTable& InnerTable) :CPrsBlock(sym), m_InnerTable(&InnerTable)
{

}
CPrsFunction::CPrsFunction(const CPrsFunction& src) :CPrsBlock(src), m_InnerTable(src.m_InnerTable)
{
	m_Value = src.m_Value;
	m_decaration = src.m_decaration;
	m_body = src.m_body;
	m_declar = src.m_declar;

	//m_decaration = std::make_shared<CPrsDecaration>(*src.m_decaration);
	//m_body = std::make_shared<CPrsBody>(*src.m_body);
	//m_declar = std::make_shared<CPrsDeclarVar>(*src.m_declar);
}
CPrsFunction::~CPrsFunction()
{
	Flush();
}
void CPrsFunction::Flush()
{
	m_Value.clear();

	m_decaration = nullptr;
	m_body = nullptr;
	m_declar = nullptr;
}
void CPrsFunction::Parse()
{
	// upperTable	: 関数を呼び出す側の変数テーブル
	// m_InnerTable:　関数内の変数テーブル


	CSymbolTable& upperTable
		= m_Symbol.SelectSymbolTable(m_InnerTable); // 関数内用のテーブルにする

	// parameterをlocalのtableへ登録
	SToken st = getSymbol();
	if (st.Token == lParen)
	{
		getNewSymbol();
		m_declar = std::make_shared<CPrsDeclarVar>(m_Symbol); // 関数のパラメータ宣言のparse
		m_declar->Parse();

		st = getSymbol();
		if (st.Token == rParen)
			getNewSymbol();
		else
			THROW(L"')' expected");
	}
	else
		THROW(L"'(' expected )");


	m_decaration = std::make_shared<CPrsDecaration>(m_Symbol);
	m_decaration->Parse();

	m_body = std::make_shared <CPrsBody>(m_Symbol);
	m_body->Parse();

	m_Symbol.SelectSymbolTable(upperTable);
}
void CPrsFunction::Generate(stackGntInfo& stinfo)
{
	// upperTable	: 関数を呼び出す側の変数テーブル
	// m_InnerTable:　関数内の変数テーブル

	CSymbolTable& upperTable
		= m_Symbol.SelectSymbolTable(m_InnerTable); // 関数内用のテーブルにする

	if (m_body)
	{
		CPrsGntInfo	gntinfo;
		stinfo.push(gntinfo);

		m_body->Generate(stinfo);
		m_Value = m_body->getValue();

		stinfo.pop();
	}
	else
		m_Value.setN(0);

	m_Symbol.SelectSymbolTable(upperTable);
}
void CPrsFunction::SetParameters(VARIANT* pv, int count)
{
	auto& lsIdent = m_declar->m_Ls;
	auto iti = lsIdent.begin();

	int i = 0;
	while (iti != lsIdent.end() && i < count)
	{
		FVariant val = pv[i++];
		wstring& localIdent = *iti;

		// 関数内用のテーブルにパラメータの値（呼出側）をセットする
		m_InnerTable.setAt(localIdent, val);
	
		val.detach(); // 重要!

		++iti;
	}
}
void CPrsFunction::SetParameters(CPrsParameters& param)
{
	// 関数のパラメータの関所
	auto& lsValue = param.getParam();
	auto& lsIdent = m_declar->m_Ls;

	auto itv = lsValue.begin();
	auto iti = lsIdent.begin();

	while (itv != lsValue.end() && iti != lsIdent.end())
	{
		FVariant& upperValue = *itv;
		wstring& localIdent = *iti;

		// 関数内用のテーブルにパラメータの値（呼出側）をセットする
		m_InnerTable.setAt(localIdent, upperValue);


		++itv;
		++iti;
	}
}

void CPrsFunction::SetParametersImport(std::map<wstring, wstring>& param)
{
	// 関数のパラメータの関所
	auto& lsIdent = m_declar->m_Ls;

	auto   iti = lsIdent.begin();
	FVariant importValue;

	while (iti != lsIdent.end())
	{
		const wstring& localIdent = *iti;
		if (param.find(localIdent) != param.end())
		{
			wstring sValue = param[localIdent];
			importValue.setS(sValue);
		}
		else
			importValue.setS(L"");

		m_InnerTable.setAt(localIdent, importValue);// 関数内用のテーブルにパラメータの値（呼出側）をセットする		

		iti++;
	}
}
void CPrsFunction::SetParametersImport(std::map<wstring, FVariant>& param)
{
	// 関数のパラメータの関所
	auto& lsIdent = m_declar->m_Ls;

	auto   iti = lsIdent.begin();
	FVariant importValue;

	while (iti != lsIdent.end())
	{
		const wstring& localIdent = *iti;
		if (param.find(localIdent) != param.end())
		{
			importValue = param[localIdent];
			//importValue.setS(sValue);
		}
		else
			importValue.setS(L"");

		m_InnerTable.setAt(localIdent, importValue);// 関数内用のテーブルにパラメータの値（呼出側）をセットする		

		iti++;
	}
}
void CPrsFunction::SetGlobalVariant(LPCWSTR prmnm, VARIANT prm)
{
	FVariant fv(prm);
	m_InnerTable.setAt(prmnm, fv);
}

