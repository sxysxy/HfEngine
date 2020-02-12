#include <ThirdParties.h>
#include <Core.h>

void HfEngineInitialize() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED); //多线程模式

    HEG::FFIInitXMMReaders();

    HEG::Window::mouse = std::make_unique<DirectX::Mouse>();
    auto& mouse = HEG::Window::mouse->Get();
    mouse.SetVisible(true);
    mouse.SetMode(DirectX::Mouse::MODE_ABSOLUTE);

    HEG::Input::Initialize();
}


int __cdecl cmain(wchar_t* path) {
    {
        using namespace Utility;
        auto ruby = ReferPtr<HEG::RubyVM>::New();

        HEG::InjectBasicExtension();  //Must be first
        HEG::InjectEasyFFIExtension();
        HEG::InjectWindowExtension();
        HEG::InjectTransformExtension();
        HEG::InjectGDeviceExtension();
        HEG::InjectCanvasExtension();
        HEG::InjectRenderContextExtension();
        auto mrb = ruby->GetRuby();
        auto ClassObject = mrb_obj_value(mrb->object_class);
        
        wchar_t buffer[MAX_PATH + 10];
        GetModuleFileNameW(GetModuleHandle(0), buffer, MAX_PATH);
        int len = lstrlenW(buffer);
        int p;
        for (p = len - 1; ~p; p--) {
            if (buffer[p] == L'\\') {
                std::string s;
                U16ToU8(buffer, s);
                for (int i = 0; i < s.length(); i++) 
                    if (s[i] == '\\')s[i] = '/';
                mrb_const_set(mrb, ClassObject, mrb_intern_lit(mrb, "EXECUTIVE_FILENAME"), 
                    mrb_str_new_cstr(mrb, s.c_str()));
                buffer[p] = 0;
                U16ToU8(buffer, s);
                for (int i = 0; i < s.length(); i++)
                    if (s[i] == '\\')s[i] = '/';
                mrb_const_set(mrb, ClassObject, mrb_intern_lit(mrb, "EXECUTIVE_DIRECTORY"),
                    mrb_str_new_cstr(mrb, s.c_str()));
                buffer[p] = L'\\';
                static const char* set_load_path = " \
                   $: ||= [] \n\
                   $: << File.join(EXECUTIVE_DIRECTORY, 'scripts') \n\
                   $: << (EXECUTIVE_DIRECTORY) \n\
                ";
                mrb_load_string(mrb, set_load_path);

                break;
            }
        }
        if (!path) {
            buffer[p + 1] = L'm';
            buffer[p + 2] = L'a';
            buffer[p + 3] = L'i';
            buffer[p + 4] = L'n';
            buffer[p + 5] = L'.';
            buffer[p + 6] = L'r';
            buffer[p + 7] = L'b';
            buffer[p + 8] = 0;
        }
        else {
            lstrcpyW(buffer, path);
            len = lstrlenW(buffer);
            for (p = len - 1; ~p; p--) {
                if (buffer[p] == L'\\')break;
            }
        }
        buffer[p] = 0;
        SetCurrentDirectory(buffer);
        buffer[p] = L'\\';
        std::string filename;
        U16ToU8(buffer, filename, CP_UTF8);
        for (int i = 0; i < filename.length(); i++)
            if (filename[i] == '\\')filename[i] = '/';
        ruby->Load(filename.c_str());
        return 0;
    }

}

wchar_t path_buffer[MAX_PATH + 10];
int HfEngineRubyEntry() {
    if (GetFileAttributes(TEXT("main.rb")) == INVALID_FILE_ATTRIBUTES) {
        if (MessageBox(0, TEXT("main.rb not found, choose a script?."), TEXT("Tip"), MB_YESNO) == IDYES) {
            OPENFILENAMEW op;
            ZeroMemory(&op, sizeof op);
            op.lStructSize = sizeof(op);
            op.lpstrFilter = L"Ruby script files(.rb)\0*.rb\0All files(*.*)\0*.*\0\0";
            op.lpstrInitialDir = L"./";
            op.lpstrFile = path_buffer;
            op.nMaxFile = MAX_PATH;
            op.nFilterIndex = 0;
            op.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            GetOpenFileNameW(&op);
            if (path_buffer[0] == 0)return 0;
            return cmain(path_buffer);
        }
        else
            return 0;
    }
    else {
        return cmain(nullptr);
    }
}
