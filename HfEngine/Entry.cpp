#include "Include/stdafx.h"
#include "Include/extension.h"
#include "Include/HFWindow.h"
#include "Include/Input.h"
#include <regex>
using namespace Utility;

void HFEngineInitialize() {
    MSVCRT::GetFunctions();
    CoInitializeEx(nullptr, COINIT_MULTITHREADED); //多线程模式
    Input::Initialize();
}

int __cdecl cmain(wchar_t *path) {
    int argc; char **argv;
    ruby_sysinit(&argc, (char***)&argv);  //这个ruby_sysinit一定要有，哪怕不用命令行参数。
    {
        RUBY_INIT_STACK;
        ruby_init();
        Ext::ApplyExtensions();

        auto buffer = ReferPtr<HFBuffer<wchar_t>>::New(MAX_PATH + 10);
        GetModuleFileNameW(GetModuleHandle(0), buffer->ptr, MAX_PATH);
        int len = lstrlenW(buffer->ptr);
        int p;
        for (p = len - 1; ~p; p--) {
            if (buffer->ptr[p] == L'\\') {
                std::string s;
                Ext::U16ToU8(buffer->ptr, s);
                rb_define_global_const("EXECUTIVE_FILENAME", rb_str_new_cstr(s.c_str()));
                s[p] = 0;
                rb_define_global_const("EXECUTIVE_DIRECTORY", rb_str_new_cstr(s.c_str()));
                static const char *require_lib = " \
                    alias :o_require :require \n \
                    def require(lib)   \n \
                        begin         \n \
                            o_require(lib) \n \
                        rescue LoadError \n \
                            o_require(EXECUTIVE_DIRECTORY+'/'+lib) \n \
                        end   \n \
                    end    \n \
                    def require_lib(lib) \n \
                        require('/libruby/'+lib)\n \
                    end \n \
                ";
                rb_eval_string(require_lib);
                break;
            }
        }
        if (!path) {
            buffer->ptr[p + 1] = L'm';
            buffer->ptr[p + 2] = L'a';
            buffer->ptr[p + 3] = L'i';
            buffer->ptr[p + 4] = L'n';
            buffer->ptr[p + 5] = L'.';
            buffer->ptr[p + 6] = L'r';
            buffer->ptr[p + 7] = L'b';
            buffer->ptr[p + 8] = 0;
        }
        else {
            lstrcpyW(buffer->ptr, path);
            len = lstrlenW(buffer->ptr);
            for (p = len - 1; ~p; p--) {
                if (buffer->ptr[p] == L'\\')break;
            }
        }
        buffer->ptr[p] = 0;
        SetCurrentDirectory(buffer->ptr);
        buffer->ptr[p] = L'\\';

        std::string filename;
        Ext::U16ToU8(buffer->ptr, filename, CP_UTF8);
        VALUE script = rb_str_new_cstr(filename.c_str());
        int state = 0;
        rb_load_protect(script, 0, &state);
        if (state) {
            VALUE errorinfo = rb_errinfo();
            rb_funcall(rb_mKernel, rb_intern("show_console"), 0);
            
            
            VALUE pos = rb_eval_string("$@");
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, pos);
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, errorinfo);
       
            rb_eval_string("STDOUT.flush");
            system("pause");
        }
        return state;
    }

}

wchar_t path_buffer[MAX_PATH + 10];
int HFEngineRubyEntry() {
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
            return cmain(path_buffer);
        }
        else
            return 0;
    }
    else {
        return cmain(nullptr);
    }
}
