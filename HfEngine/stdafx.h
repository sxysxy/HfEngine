#pragma once

#ifdef _MSC_VER
#pragma warning(disable:4005 4390)  
#endif

#include <vector>
#include <algorithm>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <ctype.h>
#include <cmath>
#include <functional>
#include <string>
#include <thread>
#include <sstream> 
#include <mutex>
#include <NativeThread.h>
#include <embeddedruby.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <winerror.h>
#include <d3d11.h>
#include <D3DX11.h>
#include <wrl\client.h>
#include <referptr.h>
#include <fpstimer.h>
#include <shapes.h>
#include <errno.h>
#include <assert.h>
#include <HFBuffer.h>
#include "libmsvcrt.h"
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#define RCHECK(x, msg) if(!(x)){MessageBoxW(0, msg, L"Error", 0); exit(0);}
#define TCHECK(x, msg) if(!(x)) {throw std::runtime_error(msg); }
#define HRCHECK(x, msg) {HRESULT hr; if(FAILED(hr = x)){throw std::runtime_error(msg); } }

template<class T>
inline void MAKE_ERRMSG(const std::string &msg, HRESULT hrcode) {
    std::stringstream _ess; 
    _ess << msg << std::hex << hrcode << '\n';
    const std::string &_esstr = _ess.str(); 
    throw T(_esstr);
}

#ifdef _UNICODE
typedef std::wstring cstring;
#else 
typedef std::string cstring;
#endif

class FileNotFoundException : public std::runtime_error {
public:
    template<class ...Arg>
    FileNotFoundException(const Arg &...arg) :std::runtime_error(arg...) {}
};

class ImplementStillNotSupported : public std::runtime_error {
public:
    template<class ...Arg>
    ImplementStillNotSupported(const Arg &...arg) :std::runtime_error(arg...) {}
};

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

#ifdef _WIN64
#define PTR_VALUE_T LONG_PTR
#else
#define PTR_VALUE_T LONG
#endif