#encoding:utf-8
require 'FFI'
include HEG

User32 = FFI::Module.new("user32.dll") {
    func :MessageBoxA, int, [int, cstr, cstr, int]
}
Kernel32 = FFI::Module.new("kernel32.dll") {
    func :AllocConsole, vptr, []
    func :GetStdHandle, vptr, [int]
    func :WriteConsoleA, int, [vptr, cstr, int, cstr, vptr]
}


user32 = FFI::dlopen("user32.dll")
kernel32 = FFI::dlopen("kernel32.dll")

User32.MessageBoxA(0, "Wow, MessageBoxA called by FFI!", "Emm", 0)
User32.MessageBoxA(0, "Looks nice, right?", "HaHa", 1)
Kernel32.AllocConsole()
std_handle = Kernel32.GetStdHandle(-11)
Kernel32.WriteConsoleA(std_handle, "HaHa, and here!\n", 16, "0"*4, 0)
Kernel32.WriteConsoleA(std_handle, "Don't consider a program with a console to be not a Win32APP!\n", 62, "0"*4, 0)
system("pause")