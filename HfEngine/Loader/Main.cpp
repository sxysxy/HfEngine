#include <Core.h>

#pragma comment(lib, "Core.lib")

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmd, int nShow) {
    using namespace Utility;
    auto window = ReferPtr<HEG::Window>::New(L"ABCD", 800, 600);
    window->Show();
    window->SetFixed(false);
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}