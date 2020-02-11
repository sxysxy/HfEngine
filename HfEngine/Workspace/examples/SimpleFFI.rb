#encoding:utf-8
include HEG
=begin
msgbox "2", "23"
user32 = FFI::dlopen("user32.dll")
MessageBox = FFI::Function.new(user32.addrof("MessageBoxA"), FFI::TYPE_INT64, 
                [FFI::TYPE_INT64, FFI::TYPE_STRING, FFI::TYPE_STRING, FFI::TYPE_INT64])
MessageBox.call(0, "MessageBoxA called by HEG::FFI", "Wow", 0)
user32.close
=end
kernel32 = FFI::dlopen("kernel32.dll")
ExitProcess = FFI::Function.new(kernel32.addrof("ExitProcess"), FFI::TYPE_INT64,
                [FFI::TYPE_INT64])
ExitProcess.call(0)
kernel32.close