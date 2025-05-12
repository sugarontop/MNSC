#pragma once
#include "CSymbolTable.h"

enum TokenType
{
	noToken, Ident, Number, beginSym, callSym,
	constSym, varSym, doSym, whileSym, endSym,	// 10
	ifSym, thenSym, endifSym, elseSym, elsifSym,
	Plus, Minus, Times, Slash, forSym,	// 20
	Percent, Equal, notEqual, LessEqual, Less,
	Greater, Becomes, Dot, IdentArray, Comma, //30
	lParen, rParen, Quotation, GreaterEqual, printSym,
	funcSym, IdentFunc, returnSym, lBracket, rBracket, //40
	objectSym, nextSym, BuiltinFunc, breakSym, IdentVarFunc,
	setSym, lSquare, rSquare, PlusEqual, Plusplus, // 50
	Minusminus, exportSym, pointSym, IdentVarPoint, IdentExport,
	ofSym, Float, IdentVarDic, expdataSym, ANDAND, // 60
	OROR, asyncSym,awaitSym,inSym, Colon,
	funcPointerSym,MinusEqual,ParameterSym,trueSym,falseSym,//70
	AssertSym,Semicol
};

typedef std::pair<wstring, int> keyword_item;


#define CR  (WCHAR)13
#define LF  (WCHAR)10
#define TAB (WCHAR)9

struct SToken
{
	int		Token;		// TokenType
	wstring	Value;
};

class CPrsFunction;

class CPrsSymbol
{
	public:
		CPrsSymbol(LPCWSTR script_text);
		~CPrsSymbol() { clear(); }

		bool Parse();

		void clear();
		WCHAR getChar();
		void ungetChar(WCHAR ch);
		const SToken& getSymbol();
		const SToken& getNewSymbol();
		const SToken getNewPeekSymbol();
		const SToken& getPreSymbol(){ return pre_token_; }
		SToken getNewSymbolReal();
		bool equal2char(const WCHAR* tknvalue);
		void setFunc(wstring funcName, std::shared_ptr<CPrsFunction> func);
		std::vector<wstring> getFuncNames();
	public:
		CSymbolTable& SelectSymbolTable(CSymbolTable& symtbl);
		CSymbolTable& getSymbolTable();
		CSymbolTable& getGlobalSymbolTable();
		
		std::shared_ptr<CPrsFunction> GetFunc(LPCWSTR funcnm);
		std::shared_ptr<CPrsFunction> CreateFunc(wstring funcName);
		
		std::shared_ptr<CPrsFunction> RegistMasterFunc(wstring funcName, bool preload=false);

		int getLineNumber() { return line_number_; }
		int getParenNumber(){ return paren_number_;}

		enum STAT { NONE, IN_PARSING };

		STAT getStat(){ return stat_;}
		STAT setStat(STAT ns){ auto old=stat_; stat_=ns; return old; }


	private :
		std::map<wstring, std::shared_ptr<CPrsFunction>>	m_funcdic;

		std::wstringstream sm_; 
		
		std::map<wstring,int> keyword_;		
		int line_number_, paren_number_;
		SToken	next_token_, pre_token_;
		bool end_;
		CSymbolTable* plocal_symtable_;
		CSymbolTable global_symtable_;
		std::queue<SToken>	queue_token_;
		STAT stat_;
};