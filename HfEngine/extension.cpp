#include "Include/stdafx.h"
#include "Include/extension.h"
#include "Include/HFWindow.h"
#include "Include/DX.h"
#include "Include\MathTool.h"

namespace Ext {

    VALUE __msgbox__(VALUE self, VALUE msg) {
        msg = rb_funcall(msg, rb_intern("to_s"), 0);
        cstring s;
        U8ToU16(rb_string_value_cstr(&msg), s);
        MessageBoxW(0, s.c_str(), L"MessageBox", MB_OK);
        return Qnil;
    }
    VALUE show_console(VALUE self) {
        HWND hwnd = FindWindow(TEXT("ConsoleWindowClass"), NULL);
        if(!hwnd)
            AllocConsole();
        else ShowWindowAsync(hwnd, SW_SHOWNORMAL);
        int s = 0;
        rb_eval_string_protect("STDIN.reopen('CON'); STDOUT.reopen('CON'); STDERR.reopen('CON');", &s);
        if (s) {
            rb_raise(rb_eException, "Error when reopening STDIO to this console.");
        }
        return Qnil;
    }
    VALUE hide_console(VALUE self) {
        HWND hwnd = FindWindow(TEXT("ConsoleWindowClass"), NULL);
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

    void BasicExtensions() {
        rb_define_module_function(rb_mKernel, "msgbox", (rubyfunc)__msgbox__, 1);
        rb_define_module_function(rb_mKernel, "show_console", (rubyfunc)show_console, 0);
        rb_define_module_function(rb_mKernel, "hide_console", (rubyfunc)hide_console, 0);
        rb_define_module_function(rb_mKernel, "exit_process", (rubyfunc)exit_process, 1);
        
    }

    VALUE messageloop(VALUE self) {
        MSG msg;
        while(true){
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD)) {
                if(msg.message == WM_EXITLOOP)break;
                //DispatchMessage(&msg);
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
                rb_funcall(rb_mKernel, rb_intern("sleep"), 1, DBL2NUM(ms / 1000.0));  //It also works
                
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


	void ApplyExtensions() {
		BasicExtensions();

		Ext::HFWindow::Init();
        rb_define_module_function(rb_mKernel, "messageloop", (rubyfunc)messageloop, 0);
        rb_define_module_function(rb_mKernel, "exit_mainloop", (rubyfunc)exit_mainloop, 0);
		Ext::DX::Init();
        Ext::FPSTimer::Init();
        Ext::MathTool::Init();

	}
}
