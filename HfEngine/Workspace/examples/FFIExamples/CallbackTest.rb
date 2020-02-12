#encoding: utf-8
CURRENT_PATH = File.dirname(__FILE__)
DLL_PATH = File.join(CURRENT_PATH, "CallbackTest.dll")
if !File.exist?(DLL_PATH)
    msgbox "Where is the dll?", "CallbackTest.dll not found in this path, See CallbackTest.txt"
    exit 1
end

include HEG
dll = FFI::dlopen(DLL_PATH)
test_closure = "Closure ok"
cb = FFI::Callback.new(FFI::TYPE_INT32, [FFI::TYPE_INT32, FFI::TYPE_INT32]) do |b, c|
    fbc = b * c
    msgbox test_closure, "get b=#{b}, c=#{c}, f(b, c) = b * c = 12"
    return fbc
end
test = FFI::Function.new(dll.addrof("test"), FFI::TYPE_INT32, [FFI::TYPE_INT32, FFI::TYPE_VOIDP])

a = 2
test.call(a, cb.addr)