#include "Include/stdafx.h"
#include "Include/extension.h"
#include "Include/HFWindow.h"
#include "Include/DX.h"
#include "Include/MathTool.h"
#include <inttypes.h>

namespace Ext {

#ifdef _DEBUG
    void CheckArgs(int argc, const VALUE *argv, const std::initializer_list<ArgType> &klasses) {
        const int xlen = argc;
        const VALUE *pobjs = argv;
        const ArgType *t = klasses.begin();
        for (int i = 0; i < xlen; i++) {
            if (t[i].nilok && NIL_P(pobjs[i]))continue;

            if (!NIL_P(pobjs[i]) && !rb_obj_is_kind_of(pobjs[i], t[i].klass)) {
                static char buf[100];

                sprintf_s(buf, "ObjectSpace._id2ref(%" PRIuPTR ").to_s", t[i].klass / 2);
                int p = 0;
                VALUE klass_name = rb_eval_string_protect(buf, &p);
                sprintf_s(buf, "ObjectSpace._id2ref(%" PRIuPTR ").to_s", rb_obj_class(pobjs[i]) / 2);

                VALUE arg_type_name = rb_eval_string_protect(buf, &p);
                rb_raise(rb_eArgError, "Param No.%d should be a %s but got a %s", i + 1, 
                    rb_string_value_cstr(&klass_name), rb_string_value_cstr(&arg_type_name));
            }
        }
    }
#endif

    VALUE rb_mModule;
    VALUE module_release;

    VALUE __msgbox__(VALUE self, VALUE msg) {
        msg = rb_funcall(msg, rb_intern("to_s"), 0);
        cstring s;
        U8ToU16(rb_string_value_cstr(&msg), s);
        MessageBoxW(0, s.c_str(), L"MessageBox", MB_OK);
        return Qnil;
    }
    HWND get_console_window() {
        HWND hwnd = FindWindow(TEXT("ConsoleWindowClass"), NULL);
        if(!hwnd)return 0;
        DWORD pid = GetCurrentProcessId();
        DWORD wpid = 0;    
        GetWindowThreadProcessId(hwnd, &wpid);
        while (wpid != pid) {
            hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
            if(!hwnd)break;
            GetWindowThreadProcessId(hwnd, &wpid);
        }
        return hwnd;
    }
    VALUE show_console(VALUE self) {
        auto hwnd = get_console_window();
        if (!hwnd) {
            AllocConsole();
            int s = 0;
            rb_eval_string_protect("STDIN.reopen('CON'); STDOUT.reopen('CON'); STDERR.reopen('CON');", &s);
            if (s) {
                rb_raise(rb_eException, "Error when reopening STDIO to this console.");
            }
        }
        else ShowWindowAsync(hwnd, SW_SHOWNORMAL);
        return Qnil;
    }
    VALUE filebox(VALUE self, VALUE title) {
        OPENFILENAMEW op;
        WCHAR path_buffer[MAX_PATH+1];
        ZeroMemory(path_buffer, sizeof(path_buffer));
        std::wstring c_title;
        U8ToU16(rb_string_value_cstr(&title), c_title);
        ZeroMemory(&op, sizeof op);
        op.lStructSize = sizeof(op);
        op.lpstrFilter = L"All files(*.*)\0*.*\0\0";
        op.lpstrInitialDir = L"./";
        op.lpstrFile = path_buffer;
        op.lpstrTitle = c_title.c_str();
        op.nMaxFile = MAX_PATH;
        op.nFilterIndex = 0;
        op.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        GetOpenFileNameW(&op);
        DWORD d = GetLastError();
        std::string s;
        U16ToU8(path_buffer, s);
        return rb_str_new(s.c_str(), s.length());
    }
    VALUE hide_console(VALUE self) {
        HWND hwnd = get_console_window();
        if (hwnd)
            ShowWindowAsync(hwnd, SW_HIDE);
        return Qnil;
    }
    VALUE dummy(VALUE self, ...) {
        return self;
    }
    VALUE exit_process(VALUE self, VALUE code) {
        ExitProcess(FIX2INT(code));
        return code;
    }
    VALUE u8ansi(VALUE self, VALUE str) {
        std::wstring tmp;
        U8ToU16(RSTRING_PTR(str), tmp);
        std::string s;
        U16ToU8(tmp.c_str(), s, CP_ACP);
        return rb_str_new(s.c_str(), s.length());
    }

    void BasicExtensions() {
        rb_mModule = CLASS_OF(rb_mMath);
        rb_define_module_function(rb_mKernel, "msgbox", (rubyfunc)__msgbox__, 1);
        rb_define_module_function(rb_mKernel, "show_console", (rubyfunc)show_console, 0);
        rb_define_module_function(rb_mKernel, "hide_console", (rubyfunc)hide_console, 0);
        rb_define_module_function(rb_mKernel, "exit_process", (rubyfunc)exit_process, 1);
        rb_define_module_function(rb_mKernel, "filebox", (rubyfunc)filebox, 1);
        rb_define_module_function(rb_mKernel, "u8ansi", (rubyfunc)u8ansi, 1);
        
    }

    //return false(ruby Qfalse) means to quit.
    //
    VALUE process_message(VALUE self) {
        static MSG msg;
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD)) {
            if (msg.message == WM_EXITLOOP)return Qfalse;
            ::HFWindow::_WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
        }
        return Qtrue;
    }

    VALUE messageloop(VALUE self) {
        MSG msg;
        while(true){
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD)) {
                if(msg.message == WM_EXITLOOP)break;
                ::HFWindow::_WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            }
            if(rb_block_given_p())
                rb_yield(Qnil);
        }
        return Qnil;
    }
    VALUE exit_mainloop(VALUE self) {
        PostThreadMessageW(GetCurrentThreadId(), WM_EXITLOOP, 0, 0);
        return Qnil;
    }

    namespace FPSTimer{
        VALUE klass;
        struct RubySleep {
            static void Wait(int ms) {
                //Sleep(ms);
                //rb_funcall(rb_mKernel, rb_intern("sleep"), 1, DBL2NUM(ms / 1000.0));  //It also works
                static struct timeval {
                    long sec;
                    long microsec;
                }t{0, 1000};  
                typedef VALUE (*rb_thread_wait_for_t)(struct timeval); 
                static rb_thread_wait_for_t rth_wait_for = (rb_thread_wait_for_t)(rb_thread_wait_for);
                    //直接用ruby.h里的rb_thread_wait_for会因为这里定义的struct timeval与ruby.h里声明的strcut timeval不是同一个东西而过不了编译...
                rth_wait_for(t);
                //if(ms)
                //    rb_thread_sleep(ms); //It preforms bad.
            }
        };
        typedef Utility::FPSTimer<RubySleep> RTimer; 

        void Delete(RTimer *t) {
            delete t;
        }
        VALUE New(VALUE klass) {
            auto t = new RTimer;
            return Data_Wrap_Struct(klass, nullptr, Delete, t);
        }
        VALUE restart(VALUE self, VALUE fps) {
            auto t = GetNativeObject<RTimer>(self);
            t->Restart(FIX2INT(fps));
            return Qnil;
        }
        VALUE await(VALUE self) {
            auto t = GetNativeObject<RTimer>(self);
            t->Await();
            return Qnil;
        }

        void Init() {
            klass = rb_define_class("FPSTimer", rb_cObject);
            rb_define_alloc_func(klass, New);
            rb_define_method(klass, "restart", (rubyfunc)restart, 1);
            rb_define_alias(klass, "initialize", "restart");
            rb_define_method(klass, "await", (rubyfunc)await, 0);
        }
    }

    static VALUE refobj_release(VALUE obj) {
        GetNativeObject<Utility::ReferredObject>(obj)->Release();
        return Qnil;
    }

	void ApplyExtensions() {
		BasicExtensions();

        module_release = rb_define_module("ReleaseObject");
        rb_define_method(module_release, "release", (rubyfunc)refobj_release, 0);

		Ext::HFWindow::Init();
        rb_define_module_function(rb_mKernel, "messageloop", (rubyfunc)messageloop, 0);
        rb_define_module_function(rb_mKernel, "process_message", (rubyfunc)process_message, 0);
        rb_define_module_function(rb_mKernel, "exit_mainloop", (rubyfunc)exit_mainloop, 0);
		Ext::DX::Init();        //初始化DX模块(包括图形和输入功能)
        Ext::FPSTimer::Init();
        Ext::MathTool::Init();  //初始化
        
	}
}
