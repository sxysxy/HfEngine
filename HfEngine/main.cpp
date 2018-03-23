﻿#include "stdafx.h"
#include "extension.h"
#include "HFWindow.h"
#include "DX\Input.h"
#include <regex>
using namespace Utility;

int __cdecl cmain(wchar_t *path) {
    int argc; char **argv;
	ruby_sysinit(&argc, (char***)&argv);  //这个ruby_sysinit一定要有，哪怕不用命令行参数。
	{
		RUBY_INIT_STACK;
		ruby_init();
		Ext::ApplyExtensions();

	    auto buffer = ReferPtr<HFBuffer<wchar_t>>::New(MAX_PATH+10);
        GetModuleFileNameW(GetModuleHandle(0), buffer->ptr, MAX_PATH);
        int len = lstrlenW(buffer->ptr);
        int p;
        for (p = len - 1; ~p; p--) {
            if (buffer->ptr[p] == L'\\')break;
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

            VALUE backtrance = rb_funcall(rb_make_backtrace(), rb_intern("to_s"), 0);
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, backtrance);
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, errorinfo);
            rb_eval_string("STDOUT.flush");
            system("pause");
		}
		return state;
	}

}

wchar_t path_buffer[MAX_PATH+10];
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nShow) {
    MSVCRT::GetFunctions();
    CoInitialize(nullptr);
    Input::Initialize();

//#define RUBY_ENTRY 
#ifdef RUBY_ENTRY
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
#else
    
#endif
    return 0;
}