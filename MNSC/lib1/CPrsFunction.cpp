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
	// upperTable	: �֐����Ăяo�����̕ϐ��e�[�u��
	// m_InnerTable:�@�֐����̕ϐ��e�[�u��


	CSymbolTable& upperTable
		= m_Symbol.SelectSymbolTable(m_InnerTable); // �֐����p�̃e�[�u���ɂ���

	// parameter��local��table�֓o�^
	SToken st = getSymbol();
	if (st.Token == lParen)
	{
		getNewSymbol();
		m_declar = std::make_shared<CPrsDeclarVar>(m_Symbol); // �֐��̃p�����[�^�錾��parse
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
	// upperTable	: �֐����Ăяo�����̕ϐ��e�[�u��
	// m_InnerTable:�@�֐����̕ϐ��e�[�u��

	CSymbolTable& upperTable
		= m_Symbol.SelectSymbolTable(m_InnerTable); // �֐����p�̃e�[�u���ɂ���

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

		// �֐����p�̃e�[�u���Ƀp�����[�^�̒l�i�ďo���j���Z�b�g����
		m_InnerTable.setAt(localIdent, val);
	
		val.detach(); // �d�v!

		++iti;
	}
}
void CPrsFunction::SetParameters(CPrsParameters& param)
{
	// �֐��̃p�����[�^�̊֏�
	auto& lsValue = param.getParam();
	auto& lsIdent = m_declar->m_Ls;

	auto itv = lsValue.begin();
	auto iti = lsIdent.begin();

	while (itv != lsValue.end() && iti != lsIdent.end())
	{
		FVariant& upperValue = *itv;
		wstring& localIdent = *iti;

		// �֐����p�̃e�[�u���Ƀp�����[�^�̒l�i�ďo���j���Z�b�g����
		m_InnerTable.setAt(localIdent, upperValue);


		++itv;
		++iti;
	}
}

void CPrsFunction::SetParametersImport(std::map<wstring, wstring>& param)
{
	// �֐��̃p�����[�^�̊֏�
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

		m_InnerTable.setAt(localIdent, importValue);// �֐����p�̃e�[�u���Ƀp�����[�^�̒l�i�ďo���j���Z�b�g����		

		iti++;
	}
}
void CPrsFunction::SetParametersImport(std::map<wstring, FVariant>& param)
{
	// �֐��̃p�����[�^�̊֏�
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

		m_InnerTable.setAt(localIdent, importValue);// �֐����p�̃e�[�u���Ƀp�����[�^�̒l�i�ďo���j���Z�b�g����		

		iti++;
	}
}
void CPrsFunction::SetGlobalVariant(LPCWSTR prmnm, VARIANT prm)
{
	FVariant fv(prm);
	m_InnerTable.setAt(prmnm, fv);
}

