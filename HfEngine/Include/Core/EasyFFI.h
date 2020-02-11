#pragma once
#include <ThirdParties.h>

#define FFI_ABI_WIN64 1
//#define FFI_ABI_WIN32_CDECL 1
//#define FFI_ABI_WIN32_STDCALL 1

HFENGINE_NAMESPACE_BEGIN

extern thread_local RClass* ModuleFFI;
extern thread_local RClass* ClassDLL;
extern thread_local RClass* ClassPointer;
extern thread_local RClass* ClassFunction;

typedef enum {
    FFI_TYPE_INT32,
    FFI_TYPE_INT64,
    FFI_TYPE_FLOAT,
    FFI_TYPE_DOUBLE,
    FFI_TYPE_STRING,
    FFI_TYPE_VOIDP,
    FFI_TYPE_UNKNOW
}FFI_CTYPE;

struct FFIFunction {

    std::vector<uint8_t> call_wrapper;

public:
    FFI_CTYPE return_type;
    std::vector<FFI_CTYPE> args_type;

    typedef union {
        int32_t as_int32;
        int64_t as_int64;
        float as_float;
        double as_double;
        const char* as_cstr;
        const void* as_vptr;

    }CallValue;
    const void* addr;
    
private:
    struct {
        int cnt_non_float;
        int cnt_float;
    }cnt;
    typedef CallValue(*CallerType)();

    void Push(int argi, CallValue &v) {
        FFI_CTYPE type = args_type[argi];
#ifdef FFI_ABI_WIN64
       
        bool pass_by_stack = false; 
        if (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT) {

        }
        else { //regard as int64
            switch (cnt.cnt_non_float++)
            {
            case 0: //mov qword rcx
                call_wrapper.push_back(0x48);
                call_wrapper.push_back(0xb9);
                break;
            case 1: //mov qword rdx
                call_wrapper.push_back(0x48);
                call_wrapper.push_back(0xba);
                break;
            case 2: //mov qword r8
                call_wrapper.push_back(0x49);
                call_wrapper.push_back(0xb8);
                break;
            case 3: //mov qword r9
                call_wrapper.push_back(0x49);
                call_wrapper.push_back(0xb9);
                break;
            default: //mov qword rax
                call_wrapper.push_back(0x48);
                call_wrapper.push_back(0xb8);
                pass_by_stack = true;
                break;
            }
            const char* pdata = reinterpret_cast<const char*>(&v.as_int64);
            for (int i = 0; i < 8; i++)
                call_wrapper.push_back(pdata[i]);
            if (pass_by_stack)
                call_wrapper.push_back(0x50); //push rax
        }
        
#endif
    }

    void GenCode(int argc, CallValue* argv) {
        call_wrapper.clear();
#ifdef FFI_ABI_WIN64
        
        //sub rsp, 0x20
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xec);
        call_wrapper.push_back(0x20);
        
#endif
        memset(&cnt, 0, sizeof(cnt));
        for (int i = 0; i < argc; i++) {
            Push(i, argv[i]);
            /*
#ifdef FFI_ABI_WIN64
            if (i == 3 || (i < 3 && i == argc - 1)) {
                unsigned char magic[] = { 0x4c, 0x89, 0x4c, 0x24, 0x20, 0x4c, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89
                    , 0x54, 0x24, 0x10, 0x48, 0x89, 0x4c, 0x24, 0x08 };
                for (int j = 0; j < ARRAYSIZE(magic); j++) {
                    call_wrapper.push_back(magic[j]);
                }
            }
#endif
            */
        }
        //mov rax, [addr]
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0xb8);
        const char* paddr = reinterpret_cast<const char*>(&addr);
        for (int i = 0; i < 8; i++)
            call_wrapper.push_back(paddr[i]);
        //call rax
        call_wrapper.push_back(0xff);
        call_wrapper.push_back(0xd0);
#ifdef FFI_ABI_WIN64
        
        //add rsp, 0x20
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xc4);
        call_wrapper.push_back(0x20);
        
        //ret
        call_wrapper.push_back(0xc3);
#endif
    }
public:

    FFIFunction(const void* addr_of_func, FFI_CTYPE ret_type, const std::vector<FFI_CTYPE>& arg_type) {
        return_type = ret_type;
        args_type = arg_type;
        addr = addr_of_func;
        call_wrapper.reserve(128);
    }
    CallValue Call(int argc, CallValue* argv) {
        GenCode(argc, argv);
        void* vpcall_wrapper = call_wrapper.data();
        DWORD old_protect = 0, unused = 0;
        VirtualProtect(vpcall_wrapper, call_wrapper.size(), PAGE_EXECUTE_READ, &old_protect);
        CallerType ct = (CallerType)vpcall_wrapper;
        CallValue ret = ct();
        VirtualProtect(vpcall_wrapper, call_wrapper.size(), old_protect, &unused);
        return ret;
    }
    
};

bool InjectEasyFFIExtension();


HFENGINE_NAMESPACE_END