#pragma once
#include <ThirdParties.h>

HFENGINE_NAMESPACE_BEGIN

extern thread_local RClass* ModuleFFI;
extern thread_local RClass* ClassDLL;
extern thread_local RClass* ClassPointer;
extern thread_local RClass* ClassFunction;

enum {
    FFI_TYPE_INT32,
    FFI_TYPE_INT64,
    FFI_TYPE_FLOAT,
    FFI_TYPE_DOUBLE,
    FFI_TYPE_STRING,
    FFI_TYPE_VOIDP,
    FFI_TYPE_UNKNOW
};

bool InjectEasyFFIExtension();


HFENGINE_NAMESPACE_END