#include <Core/Basic.h>
#include <Core/RubyVM.h>

HFENGINE_NAMESPACE_BEGIN

/*[DOCUMENT]
method: Kernel::msgbox(title : String, message : String) -> self
note: Make a message box and show(block your thread)
*/
static mrb_value Kernel_msgbox(mrb_state* mrb, mrb_value self) {
    mrb_value title_obj, msg_obj;
    mrb_get_args(mrb, "SS", &title_obj, &msg_obj);
    std::wstring titlew, msgw;
    
    U8ToU16(RSTR_PTR(mrb_str_ptr(title_obj)), titlew);
    U8ToU16(RSTR_PTR(mrb_str_ptr(msg_obj)), msgw);
    MessageBoxW(0, msgw.c_str(), titlew.c_str(), MB_OK);
    return self;
}

HWND get_console_window() {
    HWND hwnd = FindWindow(TEXT("ConsoleWindowClass"), NULL);
    if (!hwnd)return 0;
    DWORD pid = GetCurrentProcessId();
    DWORD wpid = 0;
    GetWindowThreadProcessId(hwnd, &wpid);
    while (wpid != pid) {
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
        if (!hwnd)break;
        GetWindowThreadProcessId(hwnd, &wpid);
    }
    return hwnd;
}

/*[DOCUMENT]
method: Kernel::show_console() -> self
note: Show console if show is not false(nil)
*/
mrb_value Kernel_show_console(mrb_state *mrb, mrb_value self) {
    auto hwnd = get_console_window();
    if (!hwnd) {
        AllocConsole();
        //mrb_load_string(mrb, "STDIN.reopen('CON'); STDOUT.reopen('CON'); STDERR.reopen('CON');");
    }
    else ShowWindowAsync(hwnd, SW_SHOWNORMAL);
    return self;
}
/*[DOCUMENT]
method: Kernel::hide_console -> self
note: Hide console window
*/
mrb_value Kernel_hide_console(mrb_state* mrb, mrb_value self) {
    HWND hwnd = get_console_window();
    if (hwnd)
        ShowWindowAsync(hwnd, SW_HIDE);
    return self;
}

/*[DOCUMENT]
method: Kernel::filebox(title : String) -> filename : String
note: Show a file dialog and return the selected filename. If no file was selected, it returns "".
*/
mrb_value Kernel_filebox(mrb_state *mrb, mrb_value self) {
    OPENFILENAMEW op;
    WCHAR path_buffer[MAX_PATH + 1];
    ZeroMemory(path_buffer, sizeof(path_buffer));
    mrb_value title_obj;
    mrb_get_args(mrb, "S", &title_obj);
    std::wstring titlew;
    U8ToU16(RSTR_PTR(mrb_str_ptr(title_obj)), titlew);
    ZeroMemory(&op, sizeof op);
    op.lStructSize = sizeof(op);
    op.lpstrFilter = L"All files(*.*)\0*.*\0Ruby Script Files(*.rb)\0*.*\0\0";
    op.lpstrInitialDir = L"./";
    op.lpstrFile = path_buffer;
    op.lpstrTitle = titlew.c_str();
    op.nMaxFile = MAX_PATH;
    op.nFilterIndex = 0;
    op.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    GetOpenFileNameW(&op);
    DWORD d = GetLastError();
    std::string s;
    U16ToU8(path_buffer, s);
    return mrb_str_new(mrb, s.c_str(), s.length());
}

mrb_value Kernel_puts(mrb_state* mrb, mrb_value self) {
    mrb_int argc;
    mrb_value* argv;
    mrb_get_args(mrb, "*!", &argv, &argc);
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    mrb_sym to_s = mrb_intern_cstr(mrb, "to_s");
    for (int i = 0; i < argc; i++) {
        mrb_value str_v;
        if (mrb_respond_to(mrb, argv[i], to_s))
            str_v = mrb_funcall(mrb, argv[i], "to_s", 0);
        else
            str_v = mrb_funcall(mrb, argv[i], "inspect", 0);

        std::wstring infow;
        U8ToU16(RSTR_PTR(mrb_str_ptr(str_v)), infow);
        DWORD t;
        WriteConsoleW(hStdOut, infow.c_str(), (DWORD)infow.length(), &t, nullptr);
        WriteConsoleW(hStdOut, L"\n", 1, &t, nullptr);
    }
    return self;
}

mrb_value Kernel_system(mrb_state* mrb, mrb_value self) {
    mrb_value cmd;
    mrb_get_args(mrb, "S", &cmd);
    system(RSTRING_PTR(cmd));
    return self;
}

mrb_value Kernel_print(mrb_state* mrb, mrb_value self) {
    mrb_int argc;
    mrb_value* argv;
    mrb_get_args(mrb, "*!", &argv, &argc);
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    mrb_sym to_s = mrb_intern_cstr(mrb, "to_s");
    for (int i = 0; i < argc; i++) {
        mrb_value str_v;
        if(mrb_respond_to(mrb, argv[i], to_s))
            str_v = mrb_funcall(mrb, argv[i], "to_s", 0);
        else 
            str_v = mrb_funcall(mrb, argv[i], "inspect", 0);
        
        std::wstring infow;
        U8ToU16(RSTR_PTR(mrb_str_ptr(str_v)), infow);
        DWORD t;
        WriteConsoleW(hStdOut, infow.c_str(), (DWORD)infow.length(), &t, nullptr);
    }
    return self;
}

mrb_value Kernel_p(mrb_state* mrb, mrb_value self) {
    mrb_int argc;
    mrb_value* argv;
    mrb_get_args(mrb, "*!", &argv, &argc);
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < argc; i++) {
        mrb_value str_v = mrb_funcall(mrb, argv[i], "inspect", 0);
        std::wstring infow;
        U8ToU16(RSTR_PTR(mrb_str_ptr(str_v)), infow);
        DWORD t;
        WriteConsoleW(hStdOut, infow.c_str(), (DWORD)infow.length(), &t, nullptr);
    }
    return self;
}

/*[DOCUMENT]
method: Kernel::exit_process(exit_code) -> self
note: Exit process with exit_code(kill all threads). Well, so this method authentically won't return. 
*/
mrb_value Kernel_exit_process(mrb_state* mrb, mrb_value self) {
    mrb_int code;
    mrb_get_args(mrb, "i", &code);
    ExitProcess((UINT)code);
    return self;
}

/*[DOCUMENT]
method: Kernel::mainloop {block} -> self
note: Make thread enter into mainloop.
*/
mrb_value Kernel_mainloop(mrb_state* mrb, mrb_value self) {
    MSG msg;
    mrb_value callback;
    mrb_get_args(mrb, "&!", &callback);
    mrb_value arg = mrb_ary_new(mrb);
    while(true) {
        if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        mrb_yield(mrb, callback, arg);
    }
    return self;
}

/*[DOCUMENT]
method: Kenrel::pack_floats(data : Array) -> packed_data : String
note: pack floats in array to buffer data.
*/
mrb_value Kernel_pack_floats(mrb_state* mrb, mrb_value self) {
    mrb_value ary;
    mrb_get_args(mrb, "A", &ary);
    mrb_int len = RARRAY_LEN(ary);
    mrb_value* data = RARRAY_PTR(ary);
    mrb_value res = mrb_str_buf_new(mrb, len * 4);
    float* dest = (float*)RSTRING_PTR(res);
    for (int i = 0; i < len; i++) {
        dest[i] = (float)mrb_float(data[i]);
    }
    return res;
}

/*[DOCUMENT]
method: Kenrel::pack_ints(data : Array) -> packed_data : String
note: Pack ints(32-bit integers) in array to buffer data.
*/
mrb_value Kernel_pack_ints(mrb_state* mrb, mrb_value self) {
    mrb_value ary;
    mrb_get_args(mrb, "A", &ary);
    mrb_int len = RARRAY_LEN(ary);
    mrb_value* data = RARRAY_PTR(ary);
    mrb_value res = mrb_str_buf_new(mrb, len * 4);
    int32_t* dest = (int32_t*)RSTRING_PTR(res);
    for (int i = 0; i < len; i++) {
        dest[i] = (int32_t)mrb_fixnum(data[i]);
    }
    return res;
}

/*[DOCUMENT]
method: Kernel::pack_longs(data : Array) -> packed_data : String
note: Pack longs(32-bit integers) in array to buffer data.
*/
mrb_value Kernel_pack_longs(mrb_state* mrb, mrb_value self) {
    mrb_value ary;
    mrb_get_args(mrb, "A", &ary);
    mrb_int len = RARRAY_LEN(ary);
    mrb_value* data = RARRAY_PTR(ary);
    mrb_value res = mrb_str_buf_new(mrb, len * 8);
    int64_t* dest = (int64_t*)RSTRING_PTR(res);
    for (int i = 0; i < len; i++) {
        dest[i] = mrb_fixnum(data[i]);
    }
    return res;
}

static RClass* ClassFPSTimer;
static mrb_data_type ClassFPSTimerDataType = mrb_data_type{ "GDevice", [](mrb_state* mrb, void* ptr) {
    delete ptr;
} };


/*[DOCUMENT]
method: HEG::FPSTimer::new(fps : Fixnum) -> timer : FPSTiemr
note: Create a FPSTimer and specific FPS.
*/
static mrb_value ClassFPSTimer_new(mrb_state* mrb, mrb_value klass) {
    mrb_int fps;
    mrb_get_args(mrb, "i", &fps);
    mrb_value self = mrb_obj_value(mrb_data_object_alloc(mrb, ClassFPSTimer, new Utility::SleepFPSTimer((int)fps), &ClassFPSTimerDataType));
    return self;
}

/*[DOCUMENT]
method: HEG::FPSTimter#restart(fps : Fixunm) -> self
note: Reset the FPS.
*/
static mrb_value ClassFPSTimer_restart(mrb_state* mrb, mrb_value self) {
    mrb_int fps;
    mrb_get_args(mrb, "i", &fps);
    GetNativeObject<Utility::SleepFPSTimer>(self)->Restart((int)fps);
    return self;
}

/*[DOCUMENT]
method: HEG::FPSTimter#wait(fps : Fixnum) -> self
note: Wait until next frame time.s
*/
static mrb_value ClassFPSTimer_wait(mrb_state* mrb, mrb_value self) {
    GetNativeObject<Utility::SleepFPSTimer>(self)->Await();
    return self;
}


bool InjectBasicExtension() {
    RubyVM* vm = currentRubyVM;
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "msgbox", Kernel_msgbox, MRB_ARGS_REQ(2));    
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "show_console", Kernel_show_console, MRB_ARGS_NONE());
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "hide_console", Kernel_hide_console, MRB_ARGS_NONE());
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "filebox", Kernel_filebox, MRB_ARGS_REQ(1));
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "system", Kernel_system, MRB_ARGS_REQ(1));
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "mainloop", Kernel_mainloop, MRB_ARGS_REQ(0));
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "exit_process", Kernel_exit_process, MRB_ARGS_REQ(1));

    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "puts", Kernel_puts, MRB_ARGS_ANY());
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "print", Kernel_print, MRB_ARGS_ANY());
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "p", Kernel_p, MRB_ARGS_ANY());
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "pack_floats", Kernel_pack_floats, MRB_ARGS_REQ(1));
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "pack_ints", Kernel_pack_ints, MRB_ARGS_REQ(1));
    mrb_define_module_function(vm->GetRuby(), vm->GetRuby()->kernel_module, "pack_longs", Kernel_pack_longs, MRB_ARGS_REQ(1));


    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* ClassObject = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ClassFPSTimer = mrb_define_class_under(mrb, HEG, "FPSTimer", mrb->object_class);
    mrb_define_class_method(mrb, ClassFPSTimer, "new", ClassFPSTimer_new, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassFPSTimer, "restart", ClassFPSTimer_restart, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassFPSTimer, "wait", ClassFPSTimer_wait, MRB_ARGS_NONE());

    return true;
}
HFENGINE_NAMESPACE_END