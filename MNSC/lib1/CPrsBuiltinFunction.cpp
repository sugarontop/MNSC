#include "pch.h"
#include "CPrsNode.h"
#include "FVariantArray.h"

CPrsBuiltinFunction::CPrsBuiltinFunction(CPrsSymbol& sym) :CPrsNode(sym)
{

}
CPrsBuiltinFunction::CPrsBuiltinFunction(const CPrsBuiltinFunction& src) :CPrsNode(src.m_Symbol)
{
	m_parameter = src.m_parameter;
	FuncName_ = src.FuncName_;
	Return_ = src.Return_;
}
CPrsBuiltinFunction::~CPrsBuiltinFunction()
{
	m_parameter = nullptr;
}
void CPrsBuiltinFunction::Parse()
{
	auto st = getSymbol();
	FuncName_ = st.Value;

	if (functions_map_.find(FuncName_) == functions_map_.end())
		THROW(L"not found Buildin function");

	st = getNewSymbol();
	_ASSERT( st.Token == lParen );

	m_parameter = std::make_shared<CPrsParameters>(m_Symbol);
	m_parameter->Parse();

	

	st = getSymbol();
	_ASSERT(st.Token == rParen);

}
void CPrsBuiltinFunction::Generate(stackGntInfo& stinfo)
{
	m_parameter->Generate(stinfo);

	auto ar = m_parameter->getParam();
	auto target = functions_map_[FuncName_];
	if ( 0 < ar.size())
	{				
		const FVariant* pv = &ar[0];		
		Return_ = target.func(pv, (int)ar.size());
	}
	else
		Return_ = target.func(nullptr,0);


}
void CPrsBuiltinFunction::Flush()
{
	m_parameter = nullptr;
}
//----------------------------------------------------------

std::map<wstring, CPrsBuiltinFunction::XSt> CPrsBuiltinFunction::functions_map_;

static FVariant f_sin(const FVariant* v, int cnt);
static FVariant f_cos(const FVariant* v, int cnt);
static FVariant f_log(const FVariant* v, int cnt);
static FVariant f_sqrt(const FVariant* v, int cnt);
static FVariant f_vt(const FVariant* v, int cnt);
static FVariant f_sleep(const FVariant* v, int cnt);
static FVariant f_rand(const FVariant* v, int cnt);
static FVariant f_range(const FVariant* v, int cnt);
static FVariant f_json_parse(const FVariant* v, int cnt);
static FVariant f_json_stringify(const FVariant* v, int cnt);
static FVariant f_create_struct(const FVariant* v, int cnt);
static FVariant f_dir(const FVariant* v, int cnt);
static FVariant f_str(const FVariant* v, int cnt);
static FVariant f_int(const FVariant* v, int cnt);


#define FUNCTIONS \
{0,L"sin", f_sin}, \
{1,L"cos",f_cos}, \
{2,L"log", f_log}, \
{3,L"sqrt", f_sqrt}, \
{4,L"vt", f_vt}, \
{5,L"sleep", f_sleep}, \
{6,L"rand", f_rand}, \
{7,L"range", f_range}, \
{8,L"json_parse", f_json_parse}, \
{9,L"json_stringify", f_json_stringify}, \
{10,L"create_struct", f_create_struct}, \
{11,L"dir", f_dir}, \
{12,L"str", f_str}, \
{13,L"int", f_int} \




void CPrsBuiltinFunction::Initialize(CSymbolTable& gtable)
{
	XSt f[] = { FUNCTIONS };
	for(auto& it : f)
	{
		functions_map_[it.nm] = it;
		gtable.set( it.nm, CSymbolTable::TYPE_BUILTIN_FUNC);
	}
}




static FVariant f_sin(const FVariant* v, int cnt)
{
	return FVariant(sin(v[0].getD()));
}
static FVariant f_cos(const FVariant* v, int cnt)
{
	return FVariant(cos(v[0].getD()));
}
static FVariant f_log(const FVariant* v, int cnt)
{
	return FVariant(log(v[0].getD()));
}
static FVariant f_sqrt(const FVariant* v, int cnt)
{
	return FVariant(sqrt(v[0].getD()));
}
static FVariant f_vt(const FVariant* v, int cnt)
{
	return FVariant(v[0].vt);
}
static FVariant f_sleep(const FVariant* v, int cnt)
{
	::Sleep(v[0].intVal);
	return FVariant(v[0]);
}
static FVariant f_rand(const FVariant* v, int cnt)
{
	static std::random_device rd; 
	static std::mt19937 gen(rd()); // メルセンヌ・ツイスタ 
	static std::uniform_real_distribution<double> dist(0.0, 1.0);
		
	return FVariant( (double)dist(gen));
}
static FVariant f_range(const FVariant* v, int cnt)
{
	__int64 s = 0, e=0, step=1;
	if (cnt == 3)
	{
		s = v[0].getN();
		e = v[1].getN();		
		step = v[2].getN();
	}
	else if (cnt == 2)
	{
		s = v[0].getN();
		e = v[1].getN();
	}
	else if (cnt == 1)
	{
		e = v[0].getN();
	}
	else
		THROW(L"range() err");

	std::vector<FVariant> ar;
	for(auto i=s; i<e; i+= step)
		ar.push_back(FVariant(i));

	FVariant ret;
	ret.setAr(ar);
	return ret;
}

static FVariant f_create_struct(const FVariant* v, int cnt)
{
	if (cnt != 1)
		THROW(L"create_struct() err");

	FVariant ret;
	
	ret.punkVal = new IVARIANTClassImp();
	ret.vt = VT_UNKNOWN;
	return ret;

}

static FVariant f_dir(const FVariant* v, int cnt)
{
	if (cnt != 1)
		THROW(L"dir() err");

	if (v[0].vt != VT_BSTR)
		THROW(L"dir() err");

	std::wstring s = v[0].bstrVal;

	if (s.length() > 0 && s[s.length() - 1] != L'\\')
		s += L'\\';

	FVariant ret;
	ret.bstrVal = ::SysAllocString(s.c_str());
	ret.vt = VT_BSTR;
	return ret;
}
static FVariant f_str(const FVariant* v, int cnt)
{
	if (cnt != 1)
		THROW(L"str() err");

	FVariant ret;	
	ret = v[0];

	ret.toStr();
	return ret;

}
static FVariant f_int(const FVariant* v, int cnt)
{
	if (cnt != 1)
		THROW(L"int() err");

	FVariant ret((int)0);

	if ( v[0].vt == VT_BSTR)
	{
		ret.intVal = _wtoi(v[0].bstrVal);
		ret.vt = VT_INT;
	}
	return ret;

}
//static FVariant f_assert(const FVariant* v, int cnt)
//{
//	if ( cnt == 2)
//	{
//		if (v[0] == v[1])
//			return FVariant(true);
//		else
//			throw wstring(L"assert fail, not equal");
//	}
//	else if (cnt == 3)
//	{
//		if (v[2].vt == VT_BSTR)
//		{
//			std::wstring flg = v[2].bstrVal;
//
//			if (flg == L"==" && v[0] == v[1])
//				return FVariant(true);
//			else if (flg == L"!=" && v[0] != v[1])
//				return FVariant(true);
//			else if (flg == L"<" && v[0] < v[1])
//				return FVariant(true);
//			else if (flg == L">" && v[0] > v[1])
//				return FVariant(true);
//		}
//	}
//	throw wstring(L"assert fail");
//	return FVariant(false);
//}

//////////////////////////////////////////////////////////////////////////////////////////



LPCWSTR GenStringAndDecodingQQ(LPCWSTR str, int* pcnt, std::wstring& ret)
{
	LPCWSTR p = str;
	_ASSERT(*p == '"');
	int& cnt = *pcnt;
	cnt = 0;
	p++;
	cnt++;

	std::wstringstream sm;

	bool bescapte = false;
	while (*p != '"' || bescapte == true)
	{
		if (*p == '\\' && !bescapte)
		{
			p++;
			cnt++;
			bescapte = true;
		}
		else
		{
			sm << *p++;
			cnt++;
			bescapte = false;
		}
	}
	_ASSERT(p - str == cnt);
	ret = sm.str();
	return p;
}

static FVariant f_json_parse(const FVariant* v, int cnt)
{
	if (v->vt != VT_BSTR) THROW(L"json parse err");

	enum val_type { none, string, number, floaing, bl };
	enum xmas_type { array,map_key,map_val,end_array,end_map};

	struct Xmas
	{
		xmas_type xmd;
		wstring mapkey;
		std::vector<FVariant> ar;
		std::map<wstring, FVariant> map;
	};

	LPCWSTR p = v->bstrVal;

	std::stack<std::shared_ptr<Xmas>> stack;
	wstring str, num, mapkey;
	bool str_is_null = true;
	val_type ty = none;

	auto ToFVariant = [&](const wstring& val)->FVariant
	{
		FVariant v;
		if (ty == val_type::string)
			v.setS(val.c_str());
		else if (ty == val_type::number)
			v.setN(_wtoi(val.c_str()));
		else if (ty == val_type::floaing)
			v.setD(_wtof(val.c_str()));
		else if (ty == val_type::bl)
			v.setBL( val == L"true" );
		return v;
	};

	while (*p)
	{
		while (*p <= ' ') p++;

		switch (*p)
		{
			case  '[':
			{
				Xmas x;
				x.xmd = xmas_type::array;
				stack.push(std::make_shared<Xmas>(x));
				str_is_null = true;
			}
			break;
			case  '{':
			{
				Xmas x;
				x.xmd = xmas_type::map_key;
				stack.push(std::make_shared<Xmas>(x));
				str_is_null = true;
			}
			break;
			case ',':
			{
				auto t = stack.top();

				if (t->xmd == xmas_type::array)
				{
					t->ar.push_back(ToFVariant(str));
				}
				else if (t->xmd == xmas_type::map_val)
				{
					t->map[mapkey] = ToFVariant(str);
					t->xmd = xmas_type::map_key;
				}
			
				if (t->xmd == xmas_type::end_map)
					t->xmd = xmas_type::map_key;
				else if (t->xmd == xmas_type::end_array)
					t->xmd = xmas_type::array;

			}
			break;
			case '"':
			{
				p++;
				std::wstringstream sm1;
				str.clear();
			
				while (true)
				{
					if (*p != '"')
					{
						if (*p == '\\' )
							p++;

						sm1 << *p;
					}
					else
					{
						str = sm1.str();
						str_is_null = false;
						break;
					}
					p++;
				}
				ty = val_type::string;
			}
			break;
			case ':':
			{
				if (stack.top()->xmd == xmas_type::map_key)
				{
					mapkey = str;
					stack.top()->mapkey = str;
				}
				stack.top()->xmd = xmas_type::map_val;
			}
			break;
			case ']':
			case '}':
			{
				auto t = stack.top();
				if (t->xmd == xmas_type::array)
				{
					if (!str_is_null)
						t->ar.push_back(ToFVariant(str));
					t->xmd = xmas_type::end_array;
				}
				else if (t->xmd == xmas_type::map_val)
				{			
					if (!str_is_null)
						t->map[mapkey] = ToFVariant(str);
					else
					{
						FVariant m;
						std::map<wstring,FVariant> empty_map;
						m.setMap(empty_map);
						t->map[mapkey] = m;
					}

					t->xmd = xmas_type::end_map;
				}

				stack.pop();
				FVariant v;
				if (stack.empty())
				{				
					if (t->xmd == xmas_type::end_map)
						v.setMap(t->map);
					else if (t->xmd == xmas_type::end_array)
						v.setAr(t->ar);
					else
						THROW(L"json_parse err1");

					return v;
				}
				else
				{
					auto base = stack.top();

					if (t->xmd == xmas_type::map_val || t->xmd == xmas_type::map_key)
					{
						_ASSERT(t->xmd == xmas_type::map_val || (t->xmd == xmas_type::map_key && t->map.size()==0));
						
						v.setMap(t->map);

						if (base->xmd == xmas_type::array)
						{
							base->ar.push_back(v);
							base->xmd = xmas_type::end_array;
						}
						else if (base->xmd == xmas_type::map_val)
						{
							base->map[base->mapkey] = v;
							base->xmd = xmas_type::end_map;
						}
					}
					else if (t->xmd == xmas_type::end_map)
					{
						v.setMap(t->map);
						if (base->xmd == xmas_type::map_val)
						{
							base->map[base->mapkey] = v;
							base->xmd = xmas_type::end_map;
						}
						else if (base->xmd == xmas_type::array)
						{
							base->ar.push_back(v);
							base->xmd = xmas_type::end_array;
						}

					}
					else if (t->xmd == xmas_type::array)
					{
						v.setAr(t->ar);

						if (base->xmd == xmas_type::array)
						{
							base->ar.push_back(v);
							base->xmd = xmas_type::end_array;
						}
						else if (base->xmd == xmas_type::map_val)
						{
							base->map[base->mapkey] = v;
							base->xmd = xmas_type::end_map;
						}
					}
					else if (t->xmd == xmas_type::end_array)
					{
						v.setAr(t->ar);
						if (base->xmd == xmas_type::array)
						{
							base->ar.push_back(v);
							base->xmd = xmas_type::end_array;
						}
						else if (base->xmd == xmas_type::map_val)
						{
							base->map[base->mapkey] = v;
							base->xmd = xmas_type::end_map;
						}

					}					
				}
			}
			break;

			default:
				if ('0' <= *p && *p <= '9' || *p == '.' || *p == '+' || *p == '-')
				{
					std::wstringstream sm1;
					ty = val_type::number;

					if (*p == '+' || *p == '-')
						sm1 << *p++;

					while ('0' <= *p && *p <= '9' || *p == '.')
					{
						sm1 << *p;

						if (*p == '.')
							ty = val_type::floaing;
						p++;
					}
					p--;
					str = sm1.str();
					str_is_null = false;
				}
				else if ( 't' == *p || *p =='f')
				{
					if ( 't' == *p && 'r' == *(p+1) && 'u' == *(p+2) && 'e' == *(p+3) )
					{
						str = L"true";
						str_is_null = false;
						ty = val_type::bl;
						p += 3;
					}
					else if ('f' == *p && 'a' == *(p + 1) && 'l' == *(p + 2) && 's' == *(p + 3) && 'e' == *(p + 4))
					{
						str = L"false";
						str_is_null = false;
						ty = val_type::bl;
						p += 4;
					}
					else
						THROW(L"json_parse err2");
				}
			break;
		}
		p++;
	}

	return FVariant();
}
/// json stringify ////////////////////////////////////////////////////////////////////////////////

std::wstring EncodingQQ(LPCWSTR str)
{
	std::wstringstream sm;
	LPCWSTR p = str;

	while (*p)
	{
		if (*p == L'"')
			sm << L"\\\"";
		else if (*p == '\\')
			sm << L"\\\\";
		else
			sm << *p;

		p++;
	}
	return sm.str();
}
static bool json_stringifyItem(std::wstringstream& sm,  IUnknown* p )
{
	if (dynamic_cast<IVARIANTArrayImp*>(p))
	{
		auto ar = static_cast<IVARIANTArrayImp*>(p);
		auto& std_ar = ar->ar;

		sm << L"[";

		UINT c = 0;
		for(VARIANT v : std_ar)
		{
			if (c++ != 0)
				sm << L',';

			switch (v.vt)
			{
				case VT_BSTR:
					sm << L'"' << EncodingQQ(v.bstrVal) << L'"';
					break;
				case VT_INT:
				case VT_I4:
					sm << v.intVal ;
					break;
				case VT_I8:
					sm << v.llVal ;
					break;
				case VT_R8:
					sm << v.dblVal ;
					break;
				case VT_UNKNOWN:
					if (!json_stringifyItem(sm, v.punkVal))
						return false;
					break;
				case VT_BOOL:
					sm << (v.boolVal== VARIANT_TRUE ? L"true" : L"false");
					break;
				case VT_I2:
					sm << v.iVal;
					break;
				case VT_R4:
					sm << v.fltVal;
					break;
			}
		}
		sm << L"]";

	}
	else if (dynamic_cast<IVARIANTMapImp*>(p))
	{
		auto map = static_cast<IVARIANTMapImp*>(p);
		auto& std_map = map->map;

		sm << L"{";

		UINT c = 0;

		for(auto& xit : std_map)
		{			
			const std::wstring& key = xit.first;
			FVariant& v = xit.second;

			if (c++!=0)
				sm << L',';				
			

			sm << L'"' << key << L"\":";

			switch (v.vt)
			{
				case VT_BSTR:
					sm << L'"' << EncodingQQ(v.bstrVal) << L'"';
					break;
				case VT_INT:
				case VT_I4:
					sm << v.intVal ;
					break;
				case VT_I8:
					sm << v.llVal;
					break;
				case VT_R8:
					sm << v.dblVal ;
					break;
				case VT_UNKNOWN:
					if (!json_stringifyItem(sm, v.punkVal))
						return false;
					break;
				case VT_BOOL:
					sm << (v.boolVal==VARIANT_TRUE ? L"true" : L"false");
					break;
				case VT_I2:
					sm << v.iVal;
					break;
				case VT_R4:
					sm << v.fltVal;
					break;
			}			
		}
		sm << L"}";
	}
	else
		THROW(L"jsongstringify err");

	return true;
}
static FVariant f_json_stringify(const FVariant* v, int cnt)
{
	if ( v->vt != VT_UNKNOWN ) THROW(L"stringify err");

	std::wstringstream sm;

	if (json_stringifyItem(sm, v->punkVal))
	{
		FVariant ret(sm.str().c_str());

		return ret;
	}

	THROW(L"stringify err");
}