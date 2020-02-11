#encoding:utf-8
include HEG

user32 = FFI::dlopen("user32.dll")
kernel32 = FFI::dlopen("kernel32.dll")

MessageBox = FFI::Function.new(user32.addrof("MessageBoxA"), FFI::TYPE_INT32, 
                [FFI::TYPE_INT32, FFI::TYPE_STRING, FFI::TYPE_STRING, FFI::TYPE_INT32])
ExitProcess = FFI::Function.new(kernel32.addrof("ExitProcess"), FFI::TYPE_INT32, [FFI::TYPE_INT32])
AllocConsole = FFI::Function.new(kernel32.addrof("AllocConsole"), FFI::TYPE_INT64, [])
GetStdHandle = FFI::Function.new(kernel32.addrof("GetStdHandle"), FFI::TYPE_INT64, [FFI::TYPE_INT32])
WriteConsole = FFI::Function.new(kernel32.addrof("WriteConsoleA"), FFI::TYPE_INT32, 
                [FFI::TYPE_INT64, FFI::TYPE_STRING, FFI::TYPE_INT32, FFI::TYPE_STRING, FFI::TYPE_VOIDP])
MessageBox.call(0, "Wow, MessageBoxA called by FFI!", "Emm", 0)
MessageBox.call(0, "Looks nice, right?", "HaHa", 1)
AllocConsole.call()
std_handle = GetStdHandle.call(-11)
WriteConsole.call(std_handle, "HaHa, and here!\n", 16, "0"*4, 0)
WriteConsole.call(std_handle, "Don't consider a program with a console to be not a Win32APP!\n", 62, "0"*4, 0)
system("pause")
ExitProcess.call(0)