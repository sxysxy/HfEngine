#pragma once
#include <ThirdParties.h>

#define FFI_ABI_WIN64 1
//#define FFI_ABI_WIN32_CDECL 1
//#define FFI_ABI_WIN32_STDCALL 1

HFENGINE_NAMESPACE_BEGIN

extern thread_local RClass* ModuleFFI;
extern thread_local RClass* ClassDLL;
extern thread_local RClass* ClassPointer;
extern thread_local RClass* ClassFunction, *ClassCallback;

typedef enum {
    FFI_TYPE_INT32,
    FFI_TYPE_FLOAT,
    FFI_TYPE_DOUBLE,
    FFI_TYPE_INT64,
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
    void* vpaddr;
    bool generated = false;
private:
    struct {
        int cnt_non_float;
        int cnt_float;
    }cnt;
    struct ArgPos {
        int argi;
        int offset;
    };
    std::vector<ArgPos> arg_pos;
#pragma warning(push)
#pragma warning(disable: 4229)
    typedef int64_t(*__fastcall CallerTypeInt64)();
    typedef int32_t(*__fastcall CallerTypeInt32)();
    typedef float(*__fastcall CallerTypeFloat)();
    typedef double(*__fastcall CallerTypeDouble)();
#pragma warning(pop)

#ifdef FFI_ABI_WIN64
    //Load argument or address of argument to rax
    inline void Load(int argi, CallValue& v, FFI_CTYPE type) {
        //mov rax, arg           <-> For non float number
        //mov rax, addr_of_arg   <-> For float number
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0xb8);
        const char* pdata = nullptr;
        void* p = nullptr;
        if (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT) {
            p = &v.as_double;
            pdata = reinterpret_cast<const char*>(&p);
        }
        else {
            pdata = reinterpret_cast<const char*>(&v.as_int64);
        }
        arg_pos.push_back(ArgPos{ argi, (int)call_wrapper.size() });
        for (int i = 0; i < 8; i++)
            call_wrapper.push_back(pdata[i]);
    }

    void UseRegister(int argi, int classi, CallValue& v, FFI_CTYPE type) {
        Load(argi, v, type);
        if (classi == 0) {
            if (type == FFI_TYPE_DOUBLE) { //movsd xmm0, [rax]
                call_wrapper.push_back(0xf2);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x00);
            }
            else if (type == FFI_TYPE_FLOAT) { //movss xmm0, [rax]
                call_wrapper.push_back(0xf3);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x00);
            }
            else {  //mov rcx, rax
                call_wrapper.push_back(0x48);
                call_wrapper.push_back(0x89);
                call_wrapper.push_back(0xc1);
            }
        }
        else if (classi == 1) {
            if (type == FFI_TYPE_DOUBLE) {  //movsd, xmm1, [rax]
                call_wrapper.push_back(0xf2);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x08);
            }
            else if (type == FFI_TYPE_FLOAT) { //movss, xmm1, [rax]
                call_wrapper.push_back(0xf3);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x08);
            }
            else {   //mov rdx, rax
                call_wrapper.push_back(0x48);
                call_wrapper.push_back(0x89);
                call_wrapper.push_back(0xc2);
            }
        }
        else if (classi == 2) {
            if (type == FFI_TYPE_DOUBLE) {   //movsd xmm2, [rax]
                call_wrapper.push_back(0xf2);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x10);
            }
            else if (type == FFI_TYPE_FLOAT) { //movss xmm2, [rax]
                call_wrapper.push_back(0xf3);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x10);
            }
            else {   //mov r8, rax
                call_wrapper.push_back(0x49);
                call_wrapper.push_back(0x89);
                call_wrapper.push_back(0xc0);
            }
        }
        else if (classi == 3) {
            if (type == FFI_TYPE_DOUBLE) {  //movsd xmm3, [rax]
                call_wrapper.push_back(0xf2);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x18);
            }
            else if (type == FFI_TYPE_FLOAT) {  //movss xmm3, [rax]
                call_wrapper.push_back(0xf3);
                call_wrapper.push_back(0x0f);
                call_wrapper.push_back(0x10);
                call_wrapper.push_back(0x18);
            }
            else {   //mov r9, rax
                call_wrapper.push_back(0x49);
                call_wrapper.push_back(0x89);
                call_wrapper.push_back(0xc1);
            }
        }
    }

    void Push(int argi, CallValue& v, FFI_CTYPE type) {
        Load(argi, v, type);
        //push rax
        call_wrapper.push_back(0x50); //push rax
    }
#endif

    void GenCode(int argc, CallValue* argv) {
        call_wrapper.clear();
        arg_pos.clear();
#ifdef FFI_ABI_WIN64


        //sub rsp, 0x08
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xec);
        call_wrapper.push_back(0x08);

        memset(&cnt, 0, sizeof(cnt));

        std::vector<int>float_args, non_float_args;
        for (int i = 0; i < argc; i++) {
            if (args_type[i] == FFI_TYPE_FLOAT || args_type[i] == FFI_TYPE_DOUBLE) {
                float_args.push_back(i);
            }
            else {
                non_float_args.push_back(i);
            }
        }
        int cnt_pushed = 0;
        for (int i = 0; i < argc; i++) {
            if (args_type[i] == FFI_TYPE_FLOAT || args_type[i] == FFI_TYPE_DOUBLE) {
                cnt.cnt_float += 1;
                int p = float_args[cnt.cnt_float - 1];
                if (cnt.cnt_float <= 4) {
                    UseRegister(i, cnt.cnt_float - 1, argv[p], args_type[p]);
                }
                else {
                    Push(i, argv[float_args[float_args.size() - ((size_t)cnt.cnt_float - 4)]], args_type[i]);
                    //Push(i, argv[p], args_type[p]);
                    cnt_pushed++;
                }
            }
            else {
                cnt.cnt_non_float += 1;
                int p = non_float_args[cnt.cnt_non_float - 1];
                if (cnt.cnt_non_float <= 4) {
                    UseRegister(i, cnt.cnt_non_float - 1, argv[p], args_type[p]);
                }
                else {
                    Push(i, argv[non_float_args[non_float_args.size() - ((size_t)cnt.cnt_non_float - 4)]], args_type[i]);
                    //Push(i, argv[p], args_type[p]);
                    cnt_pushed++;
                }
            }
        }
        
        //sub rsp, 0x20
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xec);
        call_wrapper.push_back(0x20);
#endif

        int sub_bytes = 0x28 + (cnt_pushed * 8);
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

        //add rsp, sub_bytes
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xc4);
        call_wrapper.push_back(sub_bytes);

        //ret
        call_wrapper.push_back(0xc3);
#endif

        vpaddr = VirtualAlloc(nullptr, call_wrapper.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (vpaddr != nullptr) {
            memcpy(vpaddr, call_wrapper.data(), call_wrapper.size());
            generated = true;
        }
    }

    //如果已经生成果代码模板，只需要填参数即可。
    void CopyArgs(int argc, CallValue* argv) {
#ifdef FFI_ABI_WIN64
        for (int i = 0; i < arg_pos.size(); i++) {
            int argi = arg_pos[i].argi;
            int offset = arg_pos[i].offset;

            const char* pdata = nullptr;
            void* p = nullptr;
            FFI_CTYPE type = args_type[argi];
            CallValue& v = argv[argi];

            if (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT) {
                p = &v.as_double;
                pdata = reinterpret_cast<const char*>(&p);
            }
            else {
                pdata = reinterpret_cast<const char*>(&v.as_int64);
            }
            memcpy((char*)vpaddr + offset, pdata, 8);
        }
#endif
    }

public:

    FFIFunction(const void* addr_of_func, FFI_CTYPE ret_type, const std::vector<FFI_CTYPE>& arg_type) {
        return_type = ret_type;
        args_type = arg_type;
        addr = addr_of_func;
        call_wrapper.reserve(128);
        generated = false;
    }
    CallValue Call(int argc, CallValue* argv) {
        if (!generated)
            GenCode(argc, argv);
        else
            CopyArgs(argc, argv);
        CallValue ret = { 0 };
        if (return_type >= FFI_TYPE_INT64 || return_type == FFI_TYPE_INT32) {
            CallerTypeInt64 ct = (CallerTypeInt64)vpaddr;
            ret.as_int64 = ct();
        }
        else if (return_type == FFI_TYPE_FLOAT) {
            CallerTypeFloat ct = (CallerTypeFloat)vpaddr;
            ret.as_float = ct();
        }
        else if (return_type == FFI_TYPE_DOUBLE) {
            CallerTypeDouble ct = (CallerTypeDouble)vpaddr;
            ret.as_double = ct();
        }
        return ret;
    }

    ~FFIFunction() {
        VirtualFree(vpaddr, call_wrapper.size(), MEM_DECOMMIT);
    }
};
#ifdef FFI_ABI_WIN64
static const uint8_t __ffi_template_callback_code[] = { 0x4c, 0x89, 0x4c, 0x24, 0x20, 0x4c, 
        0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4c, 0x24, 0x8, 0x48, 0xb9, 
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
        0x48, 0x89, 0xe2, 0x48, 0x83, 0xc2, 0x8, 0x48, 0x83, 0xec, 0x28, 0x48, 0xb8, 
        0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
        0xff, 0xd0, 0x48, 0x83, 0xc4, 0x28, 0xc3 };
/*
These binary code equals to following nasm code:
[bits 64]
mov [rsp+0x20], r9
mov [rsp+0x18], r8
mov [rsp+0x10], rdx
mov [rsp+0x08], rcx
mov rcx, 0x1111111111111111   ;Address of CallbackInfo
mov rdx, rsp
add rdx, 0x08                 ;argv
;Call Callback Implementer
sub rsp, 0x28
mov rax, 0x2222222222222222   ;Address of Callback Implementer
call rax
add rsp, 0x28
ret
*/
#endif

typedef float (*read_xmm0_float)();
extern read_xmm0_float ReadXMM0Float;
static const uint8_t __ffi_read_xmm0_float_code[] = { 0xc3 };

typedef float (*read_xmm1_float)();
extern read_xmm1_float ReadXMM1Float;
static const uint8_t __ffi_read_xmm1_float_code[] = { 0xf3, 0xf, 0x10, 0xc1, 0xc3 };

typedef float (*read_xmm2_float)();
extern read_xmm2_float ReadXMM2Float;
static const uint8_t __ffi_read_xmm2_float_code[] = { 0xf3, 0xf, 0x10, 0xc2, 0xc3 };

typedef float (*read_xmm3_float)();
extern read_xmm3_float ReadXMM3Float;
static const uint8_t __ffi_read_xmm3_float_code[] = { 0xf3, 0xf, 0x10, 0xc3, 0xc3 };

typedef double (*read_xmm0_double)();
extern read_xmm0_double ReadXMM0Double;
static const uint8_t __ffi_read_xmm0_double_code[] = { 0xc3 };

typedef double (*read_xmm1_double)();
extern read_xmm1_double ReadXMM1Double;
static const uint8_t __ffi_read_xmm1_double_code[] = { 0xf2, 0xf, 0x10, 0xc1, 0xc3 };

typedef double (*read_xmm2_double)();
extern read_xmm2_double ReadXMM2Double;
static const uint8_t __ffi_read_xmm2_double_code[] = { 0xf2, 0xf, 0x10, 0xc2, 0xc3 };

typedef double (*read_xmm3_double)();
extern read_xmm3_double ReadXMM3Double;
static const uint8_t __ffi_read_xmm3_double_code[] = { 0xf2, 0xf, 0x10, 0xc3, 0xc3 };

static void FFIInitXMMReaders() {
    char* p = (char*)VirtualAlloc(0, 5 * 6 + 2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    memcpy(p, __ffi_read_xmm0_float_code, 1);
    memcpy(p + 1, __ffi_read_xmm1_float_code, 5);
    memcpy(p + 6, __ffi_read_xmm2_float_code, 5);
    memcpy(p + 11, __ffi_read_xmm3_float_code, 5);
    memcpy(p + 16, __ffi_read_xmm0_double_code, 1);
    memcpy(p + 17, __ffi_read_xmm1_double_code, 5);
    memcpy(p + 22, __ffi_read_xmm2_double_code, 5);
    memcpy(p + 27, __ffi_read_xmm3_double_code, 5);
    ReadXMM0Float = (read_xmm0_float)p;
    ReadXMM1Float = (read_xmm1_float)(p+1);
    ReadXMM2Float = (read_xmm2_float)(p+6);
    ReadXMM3Float = (read_xmm3_float)(p+11);
    ReadXMM0Double = (read_xmm0_double)(p+16);
    ReadXMM1Double = (read_xmm1_double)(p+17);
    ReadXMM2Double = (read_xmm2_double)(p+22);
    ReadXMM3Double = (read_xmm3_double)(p+27);
}

struct FFIMRubyCallbackInfo {
    FFI_CTYPE return_type;
    int argc;
    FFI_CTYPE* arg_type;
    mrb_state* mrb;
    mrb_value mrb_proc_obj;
};

class FFICallback {
    void* vpaddr = nullptr;
    FFI_CTYPE return_type;
    std::vector<FFI_CTYPE> args_type;
    void* pcallback_info;
public:
    typedef FFIFunction::CallValue(*CallbackImplementer)(void* callback_info, FFIFunction::CallValue* argv);
private:
    CallbackImplementer cb_impl;
public:
    FFICallback(FFI_CTYPE return_val_type, const std::vector<FFI_CTYPE>& arg_type, void* callback_info, CallbackImplementer impl) {
#ifdef FFI_ABI_WIN64
        vpaddr = VirtualAlloc(0, sizeof(__ffi_template_callback_code), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        return_type = return_val_type;
        args_type = arg_type;
        pcallback_info = callback_info;
        cb_impl = impl;
        memcpy(vpaddr, __ffi_template_callback_code, sizeof(__ffi_template_callback_code));

        const char* pdata = reinterpret_cast<const char*>(&pcallback_info);

        memcpy((char*)vpaddr + 22, pdata, 8);
        pdata = reinterpret_cast<const char*>(&cb_impl);
        memcpy((char*)vpaddr + 43, pdata, 8);
#endif
    }
    ~FFICallback() {
#ifdef FFI_ABI_WIN64
        if (vpaddr)
            VirtualFree(vpaddr, sizeof(__ffi_template_callback_code), MEM_DECOMMIT);
#endif
    }
    void* CallbackAddress() const{
        return vpaddr;
    }
    FFI_CTYPE* GetArgumentsType() {
        return args_type.data();
    }
    const FFI_CTYPE* GetArgumentsType() const {
        return args_type.data();
    }
};

bool InjectEasyFFIExtension();


HFENGINE_NAMESPACE_END