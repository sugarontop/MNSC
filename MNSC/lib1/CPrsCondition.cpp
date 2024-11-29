#include "pch.h"
#include "CPrsNode.h"

class CSubExpression
{
public:
	CSubExpression(CPrsConditionEx::ComparisonNode cn, CPrsConditionEx::ComparisonNode cn2, int Token = noToken) 
	{ 
		cn_ = cn; cn2_=cn2; token = Token; 
	}
public:
	CPrsConditionEx::ComparisonNode cn_,cn2_;
	int					token;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
CPrsConditionEx::CPrsConditionEx(CPrsSymbol& sym) :CPrsNode(sym), m_bool(false)
{

}
CPrsConditionEx::CPrsConditionEx(const CPrsConditionEx& src) :CPrsNode(src.m_Symbol)
{
	m_bool = src.m_bool;
	m_Ls = src.m_Ls;
	m_Ls2 = src.m_Ls2;
}
CPrsConditionEx::~CPrsConditionEx()
{
	Flush();
}
void CPrsConditionEx::Flush()
{
	std::stack<ComparisonNode>	x;
	m_Ls.swap(x);
	m_Ls2.clear();
}
CPrsConditionEx::ComparisonNode CPrsConditionEx::Parse1()
{	
	ComparisonNode cn;
	auto expression = std::make_shared<CPrsExpression>(m_Symbol);
	expression->Parse();
	SToken st = getSymbol();
	cn.left = expression;

	if (st.Token == Equal || st.Token == notEqual || st.Token == LessEqual
		|| st.Token == Less || st.Token == Greater || st.Token == GreaterEqual)
	{
		getNewSymbol();
		expression = std::make_shared<CPrsExpression>(m_Symbol);
		expression->Parse();

		cn.operation = st.Token;
		cn.right = expression;	
		
	}
	return cn;
}

static bool opEqual(int operation, const FVariant& value, const FVariant& value2)
{
	bool bl = false;
	switch (operation)
	{
		case Equal:
			bl = (value == value2);
			break;
		case notEqual:
			bl = (value != value2);
			break;
		case LessEqual:
			bl = (value <= value2);
			break;
		case GreaterEqual:
			bl = (value >= value2);
			break;
		case Less:
			bl = (value < value2);
			break;
		case Greater:
			bl = (value > value2);
			break;
	}

	return bl;
}

void CPrsConditionEx::Generate(stackGntInfo& stinfo)
{
	if (!m_Ls.empty())
	{
		// ex. if ( a == 1 ) 
		auto x = m_Ls.top();

		x.left->Generate(stinfo);
		auto value = x.left->getValue();

		if (x.right == nullptr )
		{
			m_bool = value.getBL();
		}
		else
		{
			x.right->Generate(stinfo);
			auto value2 = x.right->getValue();
			m_bool = opEqual(x.operation, value, value2);		
		}
	}
	else
	{
		// ex. if ( a == 1 && b == 1) 
		int c = 0;
		for(auto& it : m_Ls2)
		{
			auto tk = it->token;
			std::stack<bool> st;

			CPrsConditionEx::ComparisonNode xx[2];
			xx[0] = it->cn_;
			xx[1] = it->cn2_;

			for(auto& x : xx )
			{
				if (x.left == nullptr)
					break;
				x.left->Generate(stinfo);
				auto value = x.left->getValue();
				
				if (x.right == nullptr)
				{
					bool bl = value.getBL();
					st.push(bl);
				}
				else
				{
					x.right->Generate(stinfo);
					auto value2 = x.right->getValue();

					bool bl = opEqual(x.operation, value, value2);

					st.push(bl);
				}
			}

			if (it->token == ANDAND)
			{
				bool bl = st.top(); 
				st.pop();

				if (!st.empty())
				{
					bool bl2 = st.top();
					bl = (bl&&bl2);
				}
				 
				 m_bool = ( c++ == 0 ? bl : (m_bool && bl));
			}
			else if (it->token == OROR)
			{			
				bool bl = st.top(); 
				st.pop();
			
				if (!st.empty())
				{
					bool bl2 = st.top();
					bl = (bl || bl2);
				}

				m_bool = (c++ == 0 ? bl : (m_bool || bl));
			}		
		}
	}
	
	int a =0;

}

/////////////////////////////////////////////////////////
void CPrsConditionEx::ParseBase()
{
	ComparisonNode cn = Parse1();

	m_Ls.push(cn);
}
void CPrsConditionEx::Parse()
{
	KConds();
}
void CPrsConditionEx::KConds()
{
	KCondTerm();
	KCond2();
}
void CPrsConditionEx::KCondTerm()
{
	KCondFactor();
	KCondTerm1();
}
void CPrsConditionEx::KCond2()
{
	CPrsConditionEx::ComparisonNode cn,cn2;
	SToken st = getSymbol();
	while (st.Token == OROR)
	{
		SToken st1 = getNewSymbol();

		KCondTerm();

		cn = m_Ls.top();
		m_Ls.pop();

		
		if ( !m_Ls.empty())
		{
			cn2 = m_Ls.top();
			m_Ls.pop();
		}
		

		m_Ls2.push_back(std::make_shared<CSubExpression>(cn,cn2, st.Token));
		st = getSymbol();
	}
}

void CPrsConditionEx::KCondTerm1()
{
	CPrsConditionEx::ComparisonNode cn, cn2;

	SToken st = getSymbol();
	while (st.Token == ANDAND)
	{
		SToken st1 = getNewSymbol();

		KCondFactor();

		cn = m_Ls.top();
		m_Ls.pop();

		if (!m_Ls.empty())
		{
			cn2 = m_Ls.top();
			m_Ls.pop();
		}

		m_Ls2.push_back(std::make_shared<CSubExpression>(cn,cn2, st.Token));
		st = getSymbol();
	}
}
void CPrsConditionEx::KCondFactor()
{
	SToken st = getSymbol();
	if (st.Token == lParen)
	{
		SToken st1 = getNewSymbol();
		KConds();

		auto st = getSymbol();
		if ( st.Token == rParen)
			st = getNewSymbol();
	}
	else
		KCond();
}
void CPrsConditionEx::KCond()
{
	ParseBase();
}