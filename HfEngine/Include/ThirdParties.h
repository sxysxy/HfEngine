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