#encoding :utf-8
require 'FFI'

include HEG 
User32 = FFI::Module.new("user32.dll") {
    func :MessageBoxA, int, [int, cstr, cstr, int]
    func :RegisterClassA, int, [cstr]
    func :CreateWindowExA, vptr, [int, vptr, cstr, int, int, int, int, int, vptr, vptr, vptr, vptr]
    func :ShowWindow, int, [vptr, vptr]
    func :PostQuitMessage, int, [int]
    func :LoadCursorA, vptr, [vptr, vptr]
    func :LoadIconA, vptr, [vptr, vptr]
    func :GetMessageA, int, [cstr, vptr, int, int]
    func :DispatchMessageA, int, [cstr]
    func :DefWindowProcA, int, [vptr, int, vptr, vptr]
}
Kernel32 = FFI::Module.new("kernel32.dll") {
    func :GetModuleHandleA, int64, [int64]
    func :ExitProcess, int, [int]
    func :GetLastError, int, [] 
}
GDI32 = FFI::Module.new("gdi32.dll") {
    func :GetStockObject, vptr, [vptr]
}

WNDCLASS = FFI::CStruct.new {
    int64 :style   #align to 8 bytes
    vptr :wndproc
    int :class_ex, :wnd_ex
    vptr :instance, :icon, :cursor, :brush, :menu_name, :class_name  
}

app_instance = Kernel32.GetModuleHandleA 0
wndproc = FFI::Callback.new(FFI::TYPE_INT32, [FFI::TYPE_VOIDP, FFI::TYPE_INT32, FFI::TYPE_VOIDP, FFI::TYPE_VOIDP]) do |hwnd, msg, wparam, lparam|
    if msg == 0x02 || msg == 0x10   #WM_DESTROY or WM_CLOSE
        User32.PostQuitMessage(0)
    else   
        return User32.DefWindowProcA(hwnd, msg, wparam, lparam)
    end
    return 0
end

wndclass = WNDCLASS.new 
wndclass_name = "window"
wndclass[:style] = 0x20  #CS_OWNDC
wndclass[:wndproc] = wndproc.addr
wndclass[:class_ex] = wndclass[:wnd_ex] = 0
wndclass[:brush] = GDI32.GetStockObject(0)        #WHITE_BRUSH
wndclass[:instance] = app_instance
wndclass[:cursor] = User32.LoadCursorA(0, 32512)  #IDC_ARROW
wndclass[:icon] = User32.LoadIconA(0, 32517)      #ICON_APPLICATION
wndclass[:menu_name] = 0
wndclass[:class_name] = str_ptr(wndclass_name)
if User32.RegisterClassA(wndclass.pack) == 0
    msgbox "err", "Failed to register window class, error code = #{Kernel32.GetLastError()}"
    Kernel32.ExitProcess 0
end
hwnd = User32.CreateWindowExA(0, str_ptr(wndclass_name), "Simple Windowed Win32APP by HEG::FFI", 
                                            0xcf000, 200, 200, 600, 400, 0, 0, app_instance, 0)
if hwnd == 0
    msgbox "err", "Failed to create window, error code = #{Kernel32.GetLastError()}"
    Kernel32.ExitProcess 0
end

User32.ShowWindow(hwnd, 1)
msg = "0"*40
while User32.GetMessageA(msg, 0, 0, 0) != 0
    User32.DispatchMessageA(msg)
end

Kernel32.ExitProcess 0