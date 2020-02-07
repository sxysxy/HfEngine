#include <Core.h>

#pragma comment(lib, "Core.lib")

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmd, int nShow) {
    using namespace Utility;
    HfEngineInitialize();
    auto ruby = ReferPtr<HEG::RubyVM>::New();

    HEG::InjectBasicExtension();
    HEG::InjectEasyFFIExtension();
    HEG::InjectWindowExtension();
    HEG::InjectGDeviceExtension();
    HEG::InjectCanvasExtension();
    HEG::InjectRenderContextExtension();
    
    ruby->Load("test.rb");
    return 0;
}