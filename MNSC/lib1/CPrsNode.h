#pragma once

#include "CPrsSymbol.h"
#include <stack>
#include "FVariantFunction.h"

/*
( lParen;
{ lBracket
[ lSquare
*/

class CPrsObject;
class CPrsBlock;
class CPrsFunction;
class CPrsStatment;
class CPrsAssign;
class CPrsWhile;
class CPrsFor;
class CPrsPrint;
class CSubTerm;
class CSubFactor;
class CPrsFunction;
class CPrsParameters;
class CSubExpression;
class CPrsStatmentList;
class CPrsReturn;
class CPrsBreak;
class CPrsExport;
class CPrsExportData;
class CPrsIfEx;
class CPrsExpression;
class CPrsTerm;
class CPrsFactor;
class CPrsCondition;
class CPrsConditionEx;

class CPrsGntInfo
{
	public:
		CPrsGntInfo():m_bReturn(false){}

		enum { out_loop, in_loop, outof_loop };
	public:
		FVariant			m_ReturnValue;
		bool				m_bReturn;
		std::stack<int>		m_stackInfo;
};

typedef std::stack<CPrsGntInfo>	stackGntInfo;	// Generate情報を各階層（CPrsBody単位)ごとにstackのデータとして管理する

class CPrsNode
{
	public:
		CPrsNode(CPrsSymbol& sym) :m_Symbol(sym) {};
		virtual ~CPrsNode() {}
		virtual void Parse() = 0;
		virtual void Generate(stackGntInfo& stinfo) = 0;
		virtual void Flush() = 0;
	public:
		const SToken& getSymbol() { return m_Symbol.getSymbol(); }
		const SToken& getNewSymbol() { return m_Symbol.getNewSymbol(); }
		const SToken getNewPeekSymbol() { return m_Symbol.getNewPeekSymbol(); }
	protected:
		CPrsSymbol& m_Symbol;
};
class CPrsProgram : public CPrsNode
{
	public:
		CPrsProgram(CPrsSymbol& sym);
		~CPrsProgram(){ Flush(); }
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
		FVariant Return() const;
		void SetPreGlobalVariant(LPCWSTR ident, VARIANT val);

		void Generate(stackGntInfo& stinfo, LPCWSTR funcnm,VARIANT* prms, int prmscnt);
	private :
		std::shared_ptr<CPrsBlock> blk_;
};
class CPrsBlock : public CPrsNode
{
	public:
		CPrsBlock(CPrsSymbol& sym);
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);	
		virtual void Flush();
		FVariant Return() const;
		void Generate(stackGntInfo& stinfo, LPCWSTR funcnm, VARIANT* prms, int prmscnt);
	private :
		std::shared_ptr<CPrsFunction> top_;
		FVariant return_;
};
class CPrsDecaration : public CPrsNode
{
	public:
		CPrsDecaration(CPrsSymbol& sym);
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush() {};
};
class CPrsBody : public CPrsNode
{
	public:
		CPrsBody(CPrsSymbol& sym);
		CPrsBody(const CPrsBody& src);
		~CPrsBody();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		const FVariant& getValue() { return m_ReturnValue; }
		virtual void Flush();
	protected:
		std::shared_ptr<CPrsStatmentList> m_statementlist;
		FVariant			m_ReturnValue;
};

#define ENDSYMBOL_END		1
#define ENDSYMBOL_ELSE		2
#define ENDSYMBOL_ENDIF		4
#define ENDSYMBOL_ENDWHILE	8
#define ENDSYMBOL_ENDNEXT	16
#define ENDSYMBOL_WHILEEND	32
#define ENDSYMBOL_ELSIF		64

class CPrsStatmentList : public CPrsNode
{
	public:
		CPrsStatmentList(CPrsSymbol& sym, int endsymbol);
		CPrsStatmentList(const CPrsStatmentList& src);
		~CPrsStatmentList();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
	protected:
		std::vector<std::shared_ptr<CPrsStatment>> m_ls;
		int		m_Endsymbol;

};
class CPrsStatment : public CPrsNode
{
	public:
		CPrsStatment(CPrsSymbol& sym);
		CPrsStatment(const CPrsStatment& src);
		~CPrsStatment();
		virtual void Flush();
		virtual void Parse() { HParse(); }
		virtual void Generate(stackGntInfo& stinfo) { HGenerate(stinfo); }

		void HGenerate(stackGntInfo& stinfo); // 1.2.58.14
		void HParse();
		bool IsNull(){ return (m_node.get() == nullptr); }
	protected:
		std::shared_ptr<CPrsNode> factoryParse(int nodetype, CPrsNode* src, int subnodetype);
		std::shared_ptr<CPrsNode> m_node;
		int			m_Nodetype;
		int			m_Subndtype;
		int			m_LineNumber;
};

class CPrsDeclarVar : public CPrsNode
{
	public:
		CPrsDeclarVar(CPrsSymbol& sym);
		CPrsDeclarVar(const CPrsDeclarVar& src);
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo) {};
		virtual void Flush() {};
	public:
		std::vector<wstring> m_Ls;
};

class CPrsPrint : public CPrsNode
{
	public:
		CPrsPrint(CPrsSymbol& sym);
		CPrsPrint(const CPrsPrint& sym);
		~CPrsPrint();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		void Flush();
	protected:
		wstring m_string;
		std::shared_ptr<CPrsExpression> m_expression;
};

// Expression, Term, Factor, Assign/////////////////////////////////////////////////////////////////////////////////
class CPrsFactor : public CPrsNode
{
	public:
		CPrsFactor(CPrsSymbol& sym);
		CPrsFactor(const CPrsFactor& src);
		~CPrsFactor();
		virtual void Flush();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		const FVariant getValue() { return m_Value; }

		void SetToken(int tk, FVariant d){ m_Token = tk; m_Value=d;}
	protected:
		FVariant	m_Value;
		wstring		m_IdentName;
		int			m_Token;
		std::shared_ptr<CPrsNode> m_node;
		std::shared_ptr<CPrsNode> m_dot_next;
	public :
		void TestParseFunctionPointer();
	protected:
		int ParseIdent(SToken& st);
		void ParseFunctionPointer(SToken& st);
		int  ParseIdentFunc(SToken& st);
		int  ParseBuiltinFunc(SToken& st);
		void ParseIdentSquare(SToken& st);
		void ParseNumber(SToken& st, __int64& outValue);
		void ParselParen(SToken& st);
		void ParselSquare(const SToken& st);
		void ParselBracket(const SToken& st);
		void ParseVarFunction(SToken& st);
		void ParseVarPoint(SToken& st);
		std::shared_ptr<CPrsNode> factory(CPrsNode* src, int type, LPCTSTR funcName = NULL);
};

class CPrsExpression : public CPrsNode
{
	friend class CPrsFactor;

public:
	CPrsExpression(CPrsSymbol& sym);
	CPrsExpression(const CPrsExpression& src);
	~CPrsExpression();
	virtual void Flush();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	const FVariant& getValue(){ return m_Value; }
	std::shared_ptr<CPrsNode> next_;
protected:
	FVariant		m_Value;
	bool			m_bMinus;
	std::vector<std::shared_ptr<CSubTerm>>	m_Ls;
};

class CPrsTerm : public CPrsNode
{
	public:
		CPrsTerm(CPrsSymbol& sym);
		CPrsTerm(const CPrsTerm& src);
		~CPrsTerm();
		virtual void Flush();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		const FVariant& getValue() { return m_Value; }
	protected:
		FVariant			m_Value;		
		std::vector<std::shared_ptr<CSubFactor>>	m_Ls;
};

class CPrsAssign : public CPrsNode
{
	public:
		CPrsAssign(CPrsSymbol& sym);
		CPrsAssign(const CPrsAssign& src);
		~CPrsAssign();
		virtual void Flush();
		virtual void Parse() { HParse(); }
		virtual void Generate(stackGntInfo& stinfo) { HGenerate(stinfo); }

		void HGenerate(stackGntInfo& stinfo);
		void HParse();
	protected:
		std::shared_ptr <CPrsExpression> m_expression;
		wstring			m_AssignName;
};




class CPrsParameters : public CPrsNode
{
	public:
		CPrsParameters(CPrsSymbol& sym);
		CPrsParameters(const CPrsParameters& src);
		~CPrsParameters();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
		void ParseBracket();
		void ParseParen2();
	public:
		std::vector<FVariant>& getParam();
	protected:
		std::vector<FVariant> m_lsVal;
		std::vector<std::shared_ptr<CPrsExpression>> m_lsExp;

};

class CPrsDeclarFunction : public CPrsNode
{
	public:
		CPrsDeclarFunction(CPrsSymbol& sym);
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo) {};
		virtual void Flush() {};
};
class CPrsFunction : public CPrsBlock
{
	public:		
		CPrsFunction(CPrsSymbol& sym, CSymbolTable& InnerTable);
		CPrsFunction(const CPrsFunction& src);
		~CPrsFunction();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
		const FVariant getValue() { return m_Value; }
		void	SetParameters(CPrsParameters& param);
		void	SetParametersImport(std::map<wstring,wstring>& param);
		void	SetParametersImport(std::map<wstring, FVariant>& param);
		void	SetGlobalVariant(LPCWSTR prmnm, VARIANT prm);
		void	SetParameters(VARIANT* pv, int count);
	protected:
		FVariant		m_Value;
	public:
		CSymbolTable	m_InnerTable;
	protected:
		std::shared_ptr<CPrsDecaration> m_decaration;
		std::shared_ptr <CPrsBody> m_body;
		std::shared_ptr <CPrsDeclarVar> m_declar;
};


class CPrsAssignPlusEqual : public CPrsAssign
{
	// ++,--, += 
	public:
		CPrsAssignPlusEqual(CPrsSymbol& sym);
		CPrsAssignPlusEqual(const CPrsAssignPlusEqual& src);
		~CPrsAssignPlusEqual();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
	protected:
		int				m_Token;
};

class CPrsAssignSquare : public CPrsAssign
{
	public:
		CPrsAssignSquare(CPrsSymbol& sym);
		CPrsAssignSquare(const CPrsAssignSquare& src);
		~CPrsAssignSquare();
		virtual void Flush();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
	protected:
		void ParseAutoAr();
	protected:
		std::shared_ptr<CPrsExpression> m_expressionInBracket;
		std::vector<FVariant>		m_ar;
};


class CPrsVarFunction : public CPrsNode
{
	friend class CPrsFactor;

	public:
		CPrsVarFunction(CPrsSymbol& sym);
		CPrsVarFunction(const CPrsVarFunction& src);
		~CPrsVarFunction();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
		void setgetValue(const FVariant& pvar);

		const FVariant& Return();

		static VARIANT VarInvoke(std::wstring funcnm, bool* bUpdate, VARIANT* v, int vcnt);
		void Parse2();
		void Generate2(stackGntInfo& stinfo);

	protected:
		std::shared_ptr <CPrsParameters> m_parameter;
		std::shared_ptr <CPrsExpression> m_expression;
		FVariant m_value;
		std::wstring DotFuncName_;
		//VARIANT Return_;
		FVariant Return_;
		std::shared_ptr<CPrsVarFunction> next_;
};
class CPrsBuiltinFunction : public CPrsNode
{
public:
	CPrsBuiltinFunction(CPrsSymbol& sym);
	CPrsBuiltinFunction(const CPrsBuiltinFunction& src);
	~CPrsBuiltinFunction();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	virtual void Flush();
	const FVariant& Return() { return Return_; }



	struct XSt
	{
		DWORD id;
		WCHAR nm[32];
		FVariant(*func)(const FVariant*, int cnt);
	};

	static std::map<wstring, CPrsBuiltinFunction::XSt> functions_map_;
	static void Initialize(CSymbolTable& gtable);

protected:
	std::shared_ptr <CPrsParameters> m_parameter;
	std::wstring FuncName_;
	FVariant Return_;
};

class CPrsIdentArray : public CPrsNode
{
	public:
		CPrsIdentArray(CPrsSymbol& sym);
		CPrsIdentArray(const CPrsIdentArray& src);
		~CPrsIdentArray();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
		const FVariant& Value(){ return Return_; }
		void setgetValue(const FVariant& var);

		std::shared_ptr<CPrsNode> next_;
	protected:
		std::shared_ptr<CPrsExpression> expression_;
		FVariant Return_;
		
		
};

class CPrsIdentFunctionPointer : public CPrsNode
{
public:
	CPrsIdentFunctionPointer(CPrsSymbol& sym);
	CPrsIdentFunctionPointer(const CPrsIdentFunctionPointer& src);
	~CPrsIdentFunctionPointer();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	virtual void Flush();
	const FVariant& Value() { return Return_; }
	
protected:
	wstring funcnm_;
	FVariant Return_;
};

class CPrsReturn : public CPrsNode
{
	public:
		CPrsReturn(CPrsSymbol& sym);
		CPrsReturn(const CPrsReturn& src);
		~CPrsReturn();
		virtual void Parse();
		virtual void Generate(stackGntInfo& stinfo);
		virtual void Flush();
	protected:
		std::shared_ptr<CPrsExpression> m_expression;

};



class CPrsIf : public CPrsNode
{
public:
	CPrsIf(CPrsSymbol& sym);
	CPrsIf(const CPrsIf& sym);
	~CPrsIf();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	void Flush();
protected:
	struct stThen
	{
		std::shared_ptr<CPrsConditionEx> m_condition;
		std::shared_ptr<CPrsStatmentList> m_statementlist_then;
	};

	std::vector<std::shared_ptr<stThen>>	m_conAr;
	std::shared_ptr<CPrsStatmentList> m_statementlist_else;
};

class CPrsFor : public CPrsNode
{
public:
	CPrsFor(CPrsSymbol& sym);
	CPrsFor(const CPrsFor& sym);
	~CPrsFor();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	void Flush();

	void Parse2();
protected:
	std::shared_ptr <CPrsAssign> m_ini_assign;
	std::shared_ptr <CPrsAssignPlusEqual> m_icr_assign;
	std::shared_ptr <CPrsConditionEx> m_condition;
	std::shared_ptr <CPrsStatmentList> m_statementlist;
	std::shared_ptr <CPrsExpression> m_in_expression;

	std::shared_ptr <CPrsParameters> in_parameters_;
	std::wstring in_ident_nm_, in_object_nm_;
};
class CPrsBreak : public CPrsNode
{
public:
	CPrsBreak(CPrsSymbol& sym);
	CPrsBreak(const CPrsBreak& src);
	~CPrsBreak();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	virtual void Flush(){};
};
class CPrsWhile : public CPrsNode
{
public:
	CPrsWhile(CPrsSymbol& sym);
	CPrsWhile(const CPrsWhile& sym);
	~CPrsWhile();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	void Flush();
protected:
	std::shared_ptr<CPrsConditionEx> m_condition;
	std::shared_ptr<CPrsStatmentList> m_statementlist;
};
class CPrsDo : public CPrsNode
{
public:
	CPrsDo(CPrsSymbol& sym);
	CPrsDo(const CPrsDo& sym);
	~CPrsDo();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	void Flush();
protected:
	std::shared_ptr<CPrsConditionEx> m_condition;
	std::shared_ptr<CPrsStatmentList> m_statementlist;
};

class CPrsConditionEx : public CPrsNode
{
public:
	CPrsConditionEx(CPrsSymbol& sym);
	CPrsConditionEx(const CPrsConditionEx& src);
	~CPrsConditionEx();
	void Flush();
	virtual void Parse();
	virtual void Generate(stackGntInfo& stinfo);
	bool enableProcess(){ return m_bool;}

	struct ComparisonNode
	{
		ComparisonNode() :operation(0){}
		std::shared_ptr<CPrsExpression> left;
		std::shared_ptr<CPrsExpression> right;
		int operation;		
	};
protected :
	void ParseBase();
	void KConds();
	void KCondTerm();
	void KCond2();
	void KCondTerm1();
	void KCondFactor();
	void KCond();
	ComparisonNode Parse1();

protected:
	std::stack<ComparisonNode>	m_Ls;
	std::vector<std::shared_ptr<CSubExpression>>	m_Ls2;
	bool			m_bool;
};


