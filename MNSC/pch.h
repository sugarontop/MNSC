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
#include <exception>

std::string w2a(std::wstring ws); // CPrsAssign.cpp

#define THROW(ws)	throw(std::wstring(ws))

typedef std::wstring wstring;



#ifdef _DEBUG
#define DEBUG_PARSE(A)	::OutputDebugString(L#A L"\n");
#else
#define DEBUG_PARSE(A)
#endif