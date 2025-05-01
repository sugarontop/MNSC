#include "pch.h"
#include <sstream>
#include <queue>
#include "CPrsSymbol.h"
#include "CPrsNode.h"

static keyword_item Method[] =
{
	keyword_item::pair(_T("next"),		nextSym),
	keyword_item::pair(_T("object"),	objectSym),
	keyword_item::pair(_T("BEGIN"),	beginSym),
	keyword_item::pair(_T("begin"),	beginSym),
	keyword_item::pair(_T("CALL"),		callSym),
	keyword_item::pair(_T("const"),		constSym),
	keyword_item::pair(_T("var"),		varSym),
	keyword_item::pair(_T("func"),		funcSym),
	keyword_item::pair(_T("do"),		doSym),
	keyword_item::pair(_T("else"),		elseSym),
	keyword_item::pair(_T("elsif"),	elsifSym),
	keyword_item::pair(_T("endif"),	endifSym),
	keyword_item::pair(_T("END"),		endSym),
	keyword_item::pair(_T("end"),		endSym),
	keyword_item::pair(_T("if"),		ifSym),
	keyword_item::pair(_T("then"),		thenSym),
	keyword_item::pair(_T("return"),	returnSym),
	keyword_item::pair(_T("print"),	printSym),
	keyword_item::pair(_T("while"),	whileSym),
	keyword_item::pair(_T("for"),		forSym),
	keyword_item::pair(_T("break"),	breakSym),
	keyword_item::pair(_T("set"),		setSym),
	keyword_item::pair(_T("export"),	exportSym),
	keyword_item::pair(_T("expdata"),	expdataSym),
	keyword_item::pair(_T("of"),		ofSym),
	keyword_item::pair(_T("async"),		asyncSym),
	keyword_item::pair(_T("await"),		awaitSym),
	keyword_item::pair(_T("in"),		inSym),
	keyword_item::pair(_T("true"),		trueSym),
	keyword_item::pair(_T("false"),		falseSym)
};

CPrsSymbol::CPrsSymbol(LPCWSTR script_text)
	:line_number_(1), paren_number_(0), end_(false), global_symtable_(this), plocal_symtable_(&global_symtable_), stat_(STAT::NONE)
{
	sm_ << script_text;

	for (int i = 0; i < _countof(Method); i++)
		keyword_[Method[i].first] = Method[i].second;
}

void CPrsSymbol::clear()
{
	stat_ = STAT::NONE;
}
WCHAR CPrsSymbol::getChar()
{
	WCHAR ch = 0;
	sm_.get(ch);
	if (ch == LF) line_number_++;

	return ch;
}
void CPrsSymbol::ungetChar(WCHAR ch)
{
	if (ch == LF) line_number_--;
	sm_.putback(ch);
}
const SToken& CPrsSymbol::getSymbol()
{
	return next_token_;
}
const SToken& CPrsSymbol::getNewSymbol()
{
	pre_token_ = next_token_;

	if (!queue_token_.empty())
		next_token_ = queue_token_.front();
	else
	{
		getNewSymbolReal();
		_ASSERT(!queue_token_.empty());
		next_token_ = queue_token_.front();
	}
	queue_token_.pop();


	return next_token_;
}
const SToken CPrsSymbol::getNewPeekSymbol()
{
	SToken nst;
	if (!queue_token_.empty())
		nst = queue_token_.front();
	else
	{
		getNewSymbolReal();
		_ASSERT(!queue_token_.empty());
		nst = queue_token_.front();
	}
	//queue_token_.pop();

	return nst;
}
bool CPrsSymbol::equal2char(const WCHAR* tknvalue)
{
	TCHAR ch2 = getChar();
	if (ch2 == tknvalue[1])
		return true;
	else
		ungetChar(ch2);
	return false;
}
bool CPrsSymbol::Parse()
{
	try
	{
		getNewSymbolReal();
	}
	catch(wstring err)
	{
		std::wcerr << L"error line_number:" << line_number_ << L", ";
		std::wcerr << err << std::endl;

		return false;
	}
	return true;
}
SToken CPrsSymbol::getNewSymbolReal()
{
	WCHAR ch;
	int tknType = noToken;
	std::wstringstream tknVal;

	do
	{
		ch = getChar();

		// '\0'から' 'までは制御文字なので無視
		while (_T('\0') < ch && ch <= _T(' ')) ch = getChar(); 

		// コメント扱い	
		if (ch == _T('/') && equal2char(_T("//")))
			while (!(LF == ch || ch == _T('\0'))) ch = getChar();	

		// 行の解析
		if (ch == _T('\0') || sm_.eof())
		{
			tknType = noToken;
			ungetChar(ch);
			end_ = true;
			break;
		}
		else if (_istdigit(ch))
		{
			tknType = Number;
			do
			{
				if (_istdigit(ch))
					tknVal << ch;
				else if (ch == _T('.'))
				{
					tknType = Float;
					tknVal << ch;
				}
				ch = getChar();
			} while (_istdigit(ch) || ch == _T('.'));
			ungetChar(ch);
		}
		else if (_istalpha(ch) || ch == _T('_'))
		{
			do
			{
				if (_istalpha(ch) || _istdigit(ch) || ch == _T('_'))
					tknVal << ch;
				ch = getChar();
			} 
			while (_istalpha(ch) || _istdigit(ch) || ch == _T('_'));
			ungetChar(ch);

			auto val = tknVal.str();
			keyword_item key(val, 0);

			auto it = keyword_.find(val);
			if (it != keyword_.end()) 
				tknType = (*it).second; 
			else
				tknType = Ident;

			tknVal.clear();
		}
		else if (ch == _T('"') || ch == _T('\'')) // 
		{
			WCHAR lastch = ch;
			
			WORD len = 0;
			ch = getChar();
			if (ch != lastch)
			{
				do
				{
					tknVal << ch;
					ch = getChar();
					len++;
				} while (ch != lastch && len < 1024);

				if (len == 1024)
				{				
					THROW(L"string over 1024 words");
				}
			}
			else
				tknVal << _T('\0');

			tknType = Quotation;
		}
		else if (ch == _T(';')) tknType = Semicol;
		else if (ch == _T('=') && equal2char(_T("=="))) tknType = Equal;
		else if (ch == _T('=')) tknType = Becomes;
		else if (ch == _T('(')) {tknType = lParen; ++paren_number_;}
		else if (ch == _T(')')) {tknType = rParen; --paren_number_;}
		else if (ch == _T('.')) tknType = Dot;
		else if (ch == _T(',')) tknType = Comma;
		else if (ch == CR) tknType = noToken;
		else if (ch == LF) tknType = noToken;
		else if (ch == _T('[')) tknType = lSquare;
		else if (ch == _T(']')) tknType = rSquare;
		else if (ch == _T('{')) tknType = lBracket;
		else if (ch == _T('}')) tknType = rBracket;
		else if (ch == _T(':')) tknType = Colon;
		else if (ch == _T('-') && equal2char(_T("->"))) tknType = pointSym;
		else if (ch == _T('!') && equal2char(_T("!="))) tknType = notEqual;
		else if (ch == _T('<') && equal2char(_T("<="))) tknType = LessEqual;
		else if (ch == _T('>') && equal2char(_T(">="))) tknType = GreaterEqual;
		else if (ch == _T('+') && equal2char(_T("++"))) tknType = Plusplus;
		else if (ch == _T('-') && equal2char(_T("--"))) tknType = Minusminus;
		else if (ch == _T('+') && equal2char(_T("+="))) tknType = PlusEqual;
		else if (ch == _T('-') && equal2char(_T("-="))) tknType = MinusEqual;
		else if (ch == _T('&') && equal2char(_T("&&"))) tknType = ANDAND;
		else if (ch == _T('|') && equal2char(_T("||"))) tknType = OROR;
		else if (ch == _T('+')) tknType = Plus;
		else if (ch == _T('-')) tknType = Minus;
		else if (ch == _T('*')) tknType = Times;
		else if (ch == _T('/')) tknType = Slash;
		else if (ch == _T('%')) tknType = Percent;
		else if (ch == _T('<')) tknType = Less;
		else if (ch == _T('>')) tknType = Greater;
		else if (end_) { tknType = noToken; break; }
		//else if ( _ismbblead( (UINT)ch )) getChar();
		else
		{			
			tknType = noToken;
			break;
		}
	}
	while (tknType == noToken);

	SToken st;

	st.Token = tknType;
	st.Value = tknVal.str();

	queue_token_.push(st);
	return st;
}
CSymbolTable& CPrsSymbol::SelectSymbolTable(CSymbolTable& symtbl)
{
	CSymbolTable* pold = plocal_symtable_;
	plocal_symtable_ = &symtbl;
	return *pold;
}
CSymbolTable& CPrsSymbol::getSymbolTable()
{
	return *plocal_symtable_;
}
CSymbolTable& CPrsSymbol::getGlobalSymbolTable()
{
	return global_symtable_;
}
std::shared_ptr<CPrsFunction> CPrsSymbol::RegistMasterFunc(wstring funcName, bool preload)
{
	std::shared_ptr<CPrsFunction> func;

	if ( preload == false )
	{
		func = std::make_shared<CPrsFunction>(*this, getSymbolTable());
		setFunc(funcName, func);

		m_funcdic[funcName] = func;
	}
	else
	{		
		getGlobalSymbolTable().set(funcName, CSymbolTable::TYPE_FUNC);
		m_funcdic[funcName] = func;
	}
	return func;
}
void CPrsSymbol::setFunc(wstring funcName, std::shared_ptr<CPrsFunction> func)
{
	if ( m_funcdic.find(funcName) == m_funcdic.end())
		m_funcdic[funcName] = func;
}
std::shared_ptr<CPrsFunction> CPrsSymbol::GetFunc(LPCWSTR funcnm)
{
	return m_funcdic[funcnm];
}
std::shared_ptr<CPrsFunction> CPrsSymbol::CreateFunc(wstring funcName)
{
	int type = 0;
	global_symtable_.findTable(funcName, type);
	if (type != CSymbolTable::TYPE_FUNC) return nullptr;
	
	auto master_func = m_funcdic[funcName];
	auto copyed_body = std::make_shared<CPrsFunction>(*master_func);

	return copyed_body;
}
std::vector<wstring> CPrsSymbol::getFuncNames()
{
	std::vector<wstring> ar;

	sm_.clear();
	sm_.seekg(0, std::ios::beg);

	int flg = 0;
	bool bloop = true;

	while( bloop )
	{
		std::wstringstream tknVal;
		int tknType = noToken;

		do
		{
			WCHAR ch = getChar();

			// '\0'から' 'までは制御文字なので無視
			while (_T('\0') < ch && ch <= _T(' ')) ch = getChar();

			// コメント扱い	
			if (ch == _T('/') && equal2char(_T("//")))
				while (!(LF == ch || ch == _T('\0'))) ch = getChar();

			// 行の解析
			if (ch == _T('\0') || sm_.eof())
			{
				tknType = noToken;
				ungetChar(ch);
				bloop = false;
				break;
			}
			else if (_istalpha(ch) || ch == _T('_'))
			{
				do
				{
					if (_istalpha(ch) || _istdigit(ch) || ch == _T('_'))
						tknVal << ch;
					ch = getChar();
				} 
				while (_istalpha(ch) || _istdigit(ch) || ch == _T('_'));
				ungetChar(ch);

				auto val = tknVal.str();
				keyword_item key(val, 0);

				auto it = keyword_.find(val);
				if (it != keyword_.end())
					tknType = (*it).second;
				else
					tknType = Ident;				
			}
			else
			{			
				tknType = noToken;
				break;
			}
		} 
		while (tknType == noToken);

		if (tknType == funcSym )
		{		
			flg = 1;
		}
		else if (tknType == Ident && flg == 1)
		{
			wstring nm = tknVal.str();
			ar.push_back(nm);
			flg = 0;
		}
	}

	SToken tk={};
	line_number_ = 1;
	next_token_ = pre_token_ = tk;
	end_ = false;

	sm_.clear();
	sm_.seekg(0,std::ios::beg );
	
	return ar;
}