#pragma once


#include "ReferPtr.h"
#include "FPSTimer.h"
#include "ThirdParties.h"
#include <Core/Basic.h>
#include <Core/EasyFFI.h>
#include <Core/Window.h>
#include <Core/RubyVM.h>
#include <Core/Entry.h>
#include <Core/GDevice.h>
#include <Core/RenderContext.h>
#include <Core/Input.h>

static void U8ToU16(const char* s, std::wstring& out, UINT cp = CP_UTF8) {
    size_t len = MultiByteToWideChar(cp, 0, s, (int)strlen(s), NULL, 0);
    wchar_t* t = new wchar_t[len + 1];
    MultiByteToWideChar(cp, 0, s, (int)strlen(s), t, (int)len);
    t[len] = '\0';
    out = t;
    delete t;
}

static void U16ToU8(const wchar_t* ws, std::string& out, UINT cp = CP_UTF8) {
    size_t len = WideCharToMultiByte(cp, 0, ws, (int)wcslen(ws), NULL, NULL, NULL, NULL);
    char* t = new char[len + 1];
    WideCharToMultiByte(cp, 0, ws, (int)lstrlenW(ws), t, (int)len, NULL, NULL);
    t[len] = '\0';
    out = t;
    delete t;
}

template<class T>
inline T* GetNativeObject(mrb_value value) {
    return static_cast<T*>(DATA_PTR(value));
}
template<class T>
inline static T* GetNULLPTRableNativeObject(mrb_value value) {
    return mrb_nil_p(value) ? nullptr : GetNativeObject<T>(value);
}


#define THROW_ERROR_CODE(err, msg, code) {\
char temp[512];\
sprintf_s(temp, "%s, error code = 0x%x", msg, code);\
throw err(temp);\
}