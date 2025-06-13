#include "pch.h"
#include <codecvt>
#include <regex>
#include <queue>
#include <algorithm>
#include <cwctype> 
#include "CVariantTool.h"
std::vector<std::wstring> Split(std::wstring str, std::wstring split_str)
{
	std::vector<std::wstring> ar;
	std::wregex re(split_str);
	std::wsregex_token_iterator it(str.begin(), str.end(), re, -1);
	std::wsregex_token_iterator end;

	while (it != end) {
		auto& item = *it++;
		ar.push_back(item.str());
	}
	return ar;
}
int64_t getUnixTime(int daysAgo)
{
	SYSTEMTIME st;
	FILETIME ft;
	ULARGE_INTEGER uli;

	// Œ»İ‚ÌƒVƒXƒeƒ€‚ğæ“¾
	GetSystemTime(&st);

	st.wHour = 23;
	st.wMinute = 59;
	st.wSecond = 59;
	st.wMilliseconds = 0;


	// SYSTEMTIME‚ğFILETIME‚É•ÏŠ·
	SystemTimeToFileTime(&st, &ft);

	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;

	// 1“ú = 24ŠÔ * 60•ª * 60•b = 86400•b
	const int64_t SECONDS_PER_DAY = 86400LL;

	// FILETIME‚©‚çUnix time‚Ö‚Ì•ÏŠ·
	// FILETIME: 1601”N1Œ1“ú‚©‚ç‚Ì100ƒiƒm•b’PˆÊ
	// Unix time: 1970”N1Œ1“ú‚©‚ç‚Ì•b”
	int64_t unixTime = (uli.QuadPart - 116444736000000000LL) / 10000000LL;
	return unixTime + (SECONDS_PER_DAY * daysAgo);
}

std::wstring inetStockData_YahooFinanceUri(std::wstring cd, int daysAgo)
{
	std::wstringstream sm;
	sm << L"https://query1.finance.yahoo.com/v8/finance/chart/";
	sm << cd;
	sm << L"?period1=" << getUnixTime(daysAgo) << L"&period2=" << getUnixTime(0) << L"&interval=1d";
	return sm.str();
}

std::string w2a(std::wstring ws)
{
	int nSize = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
	if (nSize > 0)
	{
		std::string str(nSize, '\0');
		WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, &str[0], nSize, NULL, NULL);
		return str;
	}
	return std::string();

}