#include <Windows.h>
#include <Entry.h>
#pragma comment(lib, "HfEngine.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmd, int nCmdShow) {
    HFEngineInitialize();
    return HFEngineRubyEntry();
}