#include "pch.h"
#include "CPrsNode.h"

// CPrsVarPoint


CPrsVarPoint::CPrsVarPoint( CPrsSymbol& sym, LPCTSTR identName ):CPrsNode( sym ),m_IdentName( identName )
{
	m_parameter		= NULL;
	m_value			= NULL; 
}
CPrsVarPoint::CPrsVarPoint( const CPrsVarPoint& src ):CPrsNode( src.m_Symbol )
{
	m_parameter		= NULL; 
	m_FuncName		= src.m_FuncName;
	m_value			= src.m_value;
	m_IdentName		= src.m_IdentName;
	if ( src.m_parameter )
		m_parameter = new CPrsParameters( *( src.m_parameter ) );
}
CPrsVarPoint::~CPrsVarPoint()
{
	Flush();
}
void CPrsVarPoint::HParse()
{
	SToken			st = getSymbol();
	CSymbolTable&	symtbl = m_Symbol.getSymbolTable();

	m_FuncName = st.Value;	// '->....' ‚ÌŠÖ”–¼
	int token = st.Token;


	if ( !m_parameter )
			m_parameter = new CPrsParameters( m_Symbol );

	st = getNewSymbol();
	if ( st.Token == lParen )
		m_parameter->Parse();
	
	if ( st.Token == lParen )
		getNewSymbol(); // ';'‚ð’Šo
}
void CPrsVarPoint::setgetValue( FVariant& var )
{
	m_value = &var;
}
void CPrsVarPoint::Flush()
{
	delete m_parameter; 
	m_parameter = NULL; 
}
void CPrsVarPoint::HGenerate( stackGntInfo& stinfo )
{
	_ASSERT( m_value );
	FVariant& Value = *m_value;

	m_parameter->Generate( stinfo );
	
	std::vector<FVariant>	ls = m_parameter->getParam();
	int param_count = ls.size();

	CPrsExportCaller* pCaller = m_Symbol.getSymbolTable().getCall();
	
#ifdef _DEBUG	
	if ( pCaller == NULL )
	{
		CString s;
		
		FVariant v1,v2,v3;
		
		if ( 2 >= ls.size() )
		{
			lstF<FVariant>::iterator it = ls.begin();
			
			v3 = *it;
			++it;
			v1 = *it;
			++it;
			v2 = *it;
			
		}
		
		
		s.Format( L"NG!!  %s %s (%d,%d)", m_IdentName.c_str(), m_FuncName.c_str(), v1.getN(), v2.getN(), v3.getSS() );
		AfxMessageBox( s );
	
	}
#endif

	// 1.2.58.37
	if( pCaller )
		*m_value = pCaller->Generate( m_IdentName, m_FuncName, ls ); 


}


