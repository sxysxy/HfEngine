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
    typedef int64_t(* __fastcall CallerTypeInt64)();
    typedef int32_t(*__fastcall CallerTypeInt32)();
    typedef float(* __fastcall CallerTypeFloat)();
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
            }else {  //mov rcx, rax
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
            else if(type == FFI_TYPE_FLOAT) { //movss, xmm1, [rax]
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

        //sub rsp, sub_bytes
        call_wrapper.push_back(0x48);
        call_wrapper.push_back(0x83);
        call_wrapper.push_back(0xec);
        call_wrapper.push_back(0x28);

#endif
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
                if (++cnt.cnt_float <= 4) {
                    UseRegister(i, cnt.cnt_float - 1, argv[i], args_type[i]);
                }
                else {
                    Push(i, argv[float_args[float_args.size() - ((size_t)cnt.cnt_float - 4)]], args_type[i]);
                    cnt_pushed++;
                }   
            }
            else {
                if (++cnt.cnt_non_float <= 4) {
                    UseRegister(i, cnt.cnt_non_float - 1, argv[i], args_type[i]);
                }
                else {
                    Push(i, argv[non_float_args[non_float_args.size() - ((size_t)cnt.cnt_non_float - 4)]], args_type[i]);
                    cnt_pushed++;
                }
            }
        }

        int sub_bytes = 0x28 + cnt_pushed * 8;

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
        if(vpaddr != nullptr) {
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
        if (return_type >= FFI_TYPE_INT64) {
            CallerTypeInt64 ct = (CallerTypeInt64)vpaddr;
            ret.as_int64 = ct();
        }
        else if (return_type == FFI_TYPE_INT32) {
            CallerTypeInt32 ct = (CallerTypeInt32)vpaddr;
            ret.as_int32 = ct();
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

bool InjectEasyFFIExtension();


HFENGINE_NAMESPACE_END