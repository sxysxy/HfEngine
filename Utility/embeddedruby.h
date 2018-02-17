#pragma once

#ifdef _WIN64

#include "C:/ruby250-x64-mswin64/include/ruby-2.5.0/ruby.h"
#pragma comment(lib, "C:/ruby250-x64-mswin64/lib/x64-vcruntime140-ruby250.lib")

#define FIX2PTR(v) NUM2ULL(v) 

#else

#include "C:/ruby250-mswin32/include/ruby-2.5.0/ruby.h"
#pragma comment(lib, "C:/ruby250-mswin32/lib/vcruntime140-ruby250.lib")
//fclose is not only in msvcrt, but also in vcruntime140-ruby250...

#define FIX2PTR(x) FIX2ULONG(x)

#endif

