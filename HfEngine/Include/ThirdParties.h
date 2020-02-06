#pragma once

#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <thread>
#include <any>
#include <filesystem>
#include <functional>
#include <memory>
#include <wrl/client.h>
#include <ReferPtr.h>
#include <FPSTimer.h>
using cstring = std::string;
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#define HFENGINE_NAMESPACE_BEGIN namespace HEG {
#define HFENGINE_NAMESPACE_END }

//d3dx11
#include <d3d11.h>
#include <D3DX11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "dxguid.lib")

//ruby
#pragma warning(push)
#pragma warning(disable: 4200)
#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/object.h>
#include <mruby/error.h>
#include <mruby/value.h>
#include <mruby/throw.h>
#include <mruby/khash.h>
#include <mruby/hash.h>
#include <regex>
extern "C" {
MRB_API void mrb_define_const(mrb_state*, struct RClass*, const char* name, mrb_value);
MRB_API mrb_value mrb_const_get(mrb_state*, mrb_value, mrb_sym);
MRB_API void mrb_const_set(mrb_state*, mrb_value, mrb_sym, mrb_value);
MRB_API int mrb_const_defined(mrb_state*, mrb_value, mrb_sym);
MRB_API int mrb_const_defined_at(mrb_state* mrb, struct RClass* klass, mrb_sym id);
MRB_API mrb_value mrb_iv_get(mrb_state* mrb, mrb_value obj, mrb_sym sym);
MRB_API void mrb_iv_set(mrb_state* mrb, mrb_value obj, mrb_sym sym, mrb_value v);
MRB_API int mrb_iv_defined(mrb_state*, mrb_value, mrb_sym);
MRB_API mrb_value mrb_gv_get(mrb_state* mrb, mrb_sym sym);
MRB_API void mrb_gv_set(mrb_state* mrb, mrb_sym sym, mrb_value val);
MRB_API mrb_value mrb_cv_get(mrb_state* mrb, mrb_value mod, mrb_sym sym);
MRB_API void mrb_cv_set(mrb_state* mrb, mrb_value mod, mrb_sym sym, mrb_value v);
MRB_API int mrb_cv_defined(mrb_state* mrb, mrb_value mod, mrb_sym sym);
}
#pragma comment(lib, "libmruby.lib")
#pragma comment(lib, "libmruby_core.lib")
#pragma comment(lib, "ws2_32.lib") //mruby needs it
#pragma warning(pop)

//SpinLock
#include <xy_async.h>

//etc
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