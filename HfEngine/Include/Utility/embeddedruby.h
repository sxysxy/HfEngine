#pragma once

#ifdef _WIN64

#include "../../../ThirdParty/ruby260-x64-mswin64/include/ruby-2.6.0/ruby.h"
#pragma comment(lib, "x64-vcruntime140-ruby260.lib")

#define FIX2PTR(v) NUM2ULL(v) 

#else

#include "../../../ThirdParty/ruby260-mswin32/include/ruby-2.6.0/ruby.h"
#pragma comment(lib, "vcruntime140-ruby260.lib")
//fclose is not only in msvcrt, but also in ruby's binary file.

#define FIX2PTR(x) FIX2ULONG(x)

#endif

