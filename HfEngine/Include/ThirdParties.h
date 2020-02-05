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
#pragma comment(lib, "libmruby.lib")
#pragma comment(lib, "libmruby_core.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(pop)