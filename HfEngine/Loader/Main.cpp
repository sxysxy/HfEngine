#include <Core/Entry.h>
#include <Windows.h>

#pragma comment(lib, "Core.lib")
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmd, int nShow) {
    HfEngineInitialize();
    return HfEngineRubyEntry();
    return 0;
}