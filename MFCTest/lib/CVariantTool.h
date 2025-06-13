
#include "mnsc.h"
#include "MsXml6.h"
#include <comutil.h>
#include <clocale>
#include <cwctype>
int64_t getUnixTime(int daysAgo);
std::vector<std::wstring> Split(std::wstring str, std::wstring split_str);
std::string w2a(std::wstring ws);

class CVARIANTTool : public IVARIANTAbstract
{
	public :
		CVARIANTTool(){};
		~CVARIANTTool(){};
		virtual void Clear(){}
		virtual int TypeId()  const { return 1001; }
	protected:		
		VARIANT inetGet(const VARIANT url)
		{			
			HRESULT hr;
			CComPtr<IXMLHTTPRequest> req;
			_bstr_t bs, headers;
						
			hr = req.CoCreateInstance(CLSID_XMLHTTP60);
			hr = req->open(
				 _bstr_t("GET"),
				 url.bstrVal,
				 _variant_t(VARIANT_FALSE),
				 _variant_t(),
				 _variant_t());

			req->setRequestHeader(_bstr_t(L"User-Agent"), _bstr_t(L"Mozilla/5.0 (compatible; AcmeInc/1.0)"));
			hr = req->send(_variant_t());
			
			long status;
			hr = req->get_status(&status);//200 : succuss
			
			VARIANT ret = MNSCCreateMap();
			IVARIANTMap* map = dynamic_cast<IVARIANTMap*>(ret.punkVal);

			if (status == 200)
			{				
				req->get_responseText(bs.GetAddress());

				req->getAllResponseHeaders(headers.GetAddress());

				std::wstring cheaders = (LPCWSTR)headers;
				std::wstring split = L"\r\n";

				std::vector<std::wstring> ar = Split(cheaders, split);

				for(auto& it : ar)
				{
					auto xar = Split(it,L":");

					if ( xar.size() >= 2 )
					{
						_variant_t v(xar[1].c_str());

						auto v1 = v.Detach();
						map->SetItem(xar[0].c_str(),v1 );
					}
				}

				auto v1 = _variant_t(status).Detach();
				map->SetItem(L"result", v1);
				v1 = _variant_t(bs).Detach();
				map->SetItem(L"json", v1);
				

				//FILE* pf;
				//std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				//CStringA strA((LPCWSTR)bs);
				//::fopen_s(&pf, "test.json", "w");
				//std::string utf8 = convert.to_bytes(bs);
				//::fwrite((LPSTR)(LPCSTR)utf8.c_str(), 1, utf8.length(), pf);
				//fclose(pf);

				return ret;
			}
			 else
			 {
				auto v1 = _variant_t(status).Detach();
				map->SetItem(L"result",v1 );

			 }

			return ret;
		}

		VARIANT Value(const VARIANT obj, const VARIANT query)
		{
			if (obj.vt != VT_UNKNOWN || query.vt != VT_BSTR)
				THROW(L"Query err");

			LPCWSTR p = query.bstrVal;

			struct st
			{
				st(int a) :typ(a), keyN(0) {}
				int typ;
				std::wstring keyS;
				UINT keyN;

			};

			std::queue< std::shared_ptr<st>> qu;
			while (*p)
			{
				while (*p <= ' ') p++;

				if (*p == '"')
				{
					auto x = std::make_shared<st>(2);
					qu.push(x);

					p++;
					std::wstringstream sm;
					while (*p != '"')
					{
						sm << *p;
						p++;
					}
					x->keyS = sm.str();
				}
				else if ('0' <= *p && *p <= '9')
				{
					auto x = std::make_shared<st>(1);
					qu.push(x);

					std::wstringstream sm;
					while ('0' <= *p && *p <= '9')
					{
						sm << *p++;
					}
					int idx = _wtoi(sm.str().c_str());
					x->keyN = idx;
					p--;
				}

				p++;
			}

			IUnknown* target = obj.punkVal;
			_variant_t ret;
			while (1)
			{
				_variant_t v;

				auto st = qu.front();
				if (st->typ == 1)
				{
					auto ar = dynamic_cast<IVARIANTArray*>(target);
					if (ar == nullptr)
						goto err;

					auto idx = st->keyN;

					if (idx < ar->Count())
						ar->Get(idx, &v);
					else
						goto err;
				}
				else if (st->typ == 2)
				{
					auto map = dynamic_cast<IVARIANTMap*>(target);
					if (map == nullptr)
						goto err;

					if (!map->GetItem(st->keyS.c_str(), &v))
						goto err;
				}

				qu.pop();

				if (v.vt != VT_UNKNOWN || qu.empty())
				{
					ret = v;
					break;
				}

				target = v.punkVal;
			}
		err:
			return ret.Detach();
		}

		VARIANT OpenTextFile(const VARIANT filename)
		{
			_variant_t ret;
			CFile cf;
			if (filename.vt == VT_BSTR && cf.Open(filename.bstrVal, CStdioFile::modeRead))
			{
				char cb[256]={};
				std::stringstream sm;
				while( cf.Read(cb, 255))
				{					
					sm << cb;
					memset(cb, 0, 256);
				}
				cf.Close();

				auto cstr = sm.str();
				int len = ::MultiByteToWideChar(CP_UTF8,0,cstr.c_str(),-1,NULL,NULL);
				std::vector<WCHAR> ar(len+1);
				::MultiByteToWideChar(CP_UTF8, 0, cstr.c_str(), -1,&ar[0], len);

				BSTR cstr2 = ::SysAllocString(&ar[0]);

				ret.vt = VT_BSTR;
				ret.bstrVal= cstr2;
			}

			return ret.Detach();
		}
		VARIANT SaveTextFile(const VARIANT filename, VARIANT text)
		{
			_variant_t ret(FALSE);
			CFile cf;
			if (filename.vt == VT_BSTR && cf.Open(filename.bstrVal, CStdioFile::modeCreate|CStdioFile::modeWrite))
			{
				
				if ( text.vt == VT_BSTR )
				{
					auto s = w2a(text.bstrVal);
					cf.Write(s.c_str(), s.length());
					cf.Close();

				}
				
				ret = _variant_t(TRUE);
			}

			return ret.Detach();
		}

	public:
		virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
			if (riid == IID_IUnknown) {
				*ppv = static_cast<IUnknown*>(this);
			}
			else {
				*ppv = nullptr;
				return E_NOINTERFACE;
			}
			AddRef();
			return S_OK;
		}
		virtual VARIANT Invoke(LPCWSTR cfuncnm, VARIANT* prm, int vcnt) override
		{
			std::wstring funcnm = cfuncnm;
			std::transform(funcnm.begin(), funcnm.end(), funcnm.begin(),
				[](wchar_t c) { return std::towlower(c); });


			if (vcnt == 1 && funcnm == L"inetget")
			{				
				VARIANT url = prm[0];

				return inetGet(url);
			}
			else if (vcnt == 2 && funcnm == L"value")
			{
				return Value(prm[0],prm[1]);
			}
			else if (vcnt == 1 && funcnm == L"read_file_utf8")
			{
				return OpenTextFile(prm[0]);
			}
			else if (vcnt == 1 && funcnm == L"loadutf8")
			{
				return OpenTextFile(prm[0]);
			}
			else if (vcnt == 1 && funcnm == L"unixtime")
			{
				int off = prm[0].intVal;
				
				auto t = getUnixTime( off );
				VARIANT r;
				VariantInit(&r);
				r.llVal = t;
				r.vt = VT_I8;
				return r;

			}
			else if (vcnt == 2 && funcnm == L"save_file")
			{
				return SaveTextFile(prm[0], prm[1]);
			}

			_variant_t ret;
			return ret;

		}
		
};


