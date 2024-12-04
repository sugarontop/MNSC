#pragma once

#include <string>
#include <map>
#include <vector>

#include <strsafe.h>
#include <atlbase.h>

#include <iostream>
#include <sstream>
#include <stack>
#include <queue>
#include <memory>
#include <functional>
#include <random>
#include <initguid.h>

#define THROW(s)	throw(std::wstring(s))
typedef std::wstring wstring;



#ifdef _DEBUG
#define DEBUG_PARSE(A)	::OutputDebugString(L#A L"\n");
#else
#define DEBUG_PARSE(A)
#endif