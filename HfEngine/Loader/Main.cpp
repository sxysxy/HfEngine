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
    FILE* fp = nullptr;
    try {
        fopen_s(&fp, "test.rb", "r");
        mrb_load_file(ruby->GetRuby(), fp);
       
    } catch(std::exception &err) {
        mrb_raise(ruby->GetRuby(), ruby->GetRuby()->eStandardError_class, err.what());
    }
    if (fp)
        fclose(fp);
    if (ruby->DealException()) {
        system("pause");
    }

    return 0;
}