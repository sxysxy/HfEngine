#include <Core/RubyVM.h>
#include <ThirdParties.h>
#include <Core/EasyFFI.h>

HFENGINE_NAMESPACE_BEGIN

thread_local RClass* ModuleFFI;
thread_local RClass* ClassDLL;
thread_local RClass* ClassPointer;
thread_local RClass* ClassFunction;

mrb_data_type ClassDLLDataType = mrb_data_type{ "DLL", [](mrb_state* mrb, void* ptr) {
} };

mrb_data_type ClassPointerDataType = mrb_data_type{ "Pointer", [](mrb_state* mrb, void* ptr) {
} };

/*[DOCUMENT]
method: HEG::FFI::dlopen(dlname : String) -> dll : DLL
note: Create an instance of DLL
*/
mrb_value FFI_dlopen(mrb_state* mrb, mrb_value self) {
    std::wstring filename;
    mrb_value filename_obj;
    mrb_get_args(mrb, "S", &filename_obj);
    U8ToU16(RSTRING_PTR(filename_obj), filename);

    if (GetFileAttributesW(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
        wchar_t syspath_buf[MAX_PATH];
        GetSystemDirectoryW(syspath_buf, MAX_PATH - 2);
        std::wstring syspath(syspath_buf);
        syspath.push_back(L'\\');
        filename = syspath + filename;
        if (GetFileAttributesW(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
            mrb_raise(mrb, mrb->eStandardError_class,
                (std::string("Could not find ") + std::string(RSTRING_PTR(filename_obj)) + std::string(" in working or system directory.")).c_str());
            return mrb_fixnum_value(0);
        }
    }
    HANDLE h = LoadLibraryW(filename.c_str());
    if (!h) {
        mrb_raise(mrb, mrb->eStandardError_class,
            (std::string("Could not load file ") + std::string(RSTRING_PTR(filename_obj)) + std::string(", maybe it is not a shared library.")).c_str());
    }
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassDLL, h, &ClassDLLDataType));
}

/*[DOCUMENT]
method: HEG::FFI::DLL#addrof(extern_symbol : String) -> address : Fixnum
note: Get address of given function in the dll, if not exist, raise Exception
*/
mrb_value ClassDLL_addrof(mrb_state* mrb, mrb_value self) {
    mrb_value sym;
    mrb_get_args(mrb, "S", &sym);
    const char* psym = RSTRING_PTR(sym);
    HMODULE hDll = (HMODULE)DATA_PTR(self);
    void* addr = GetProcAddress(hDll, psym);
    if (addr == 0) {
        mrb_raise(mrb, mrb->eStandardError_class, (std::string("Could not get address of ") + RSTRING_PTR(sym)).c_str());
        return mrb_fixnum_value(0);
    }
    return mrb_fixnum_value((mrb_int)addr);
}

/*[DOCUMENT]
method: HEG::FFI::DLL#to_i -> handle : Integer
note: Get the HANDLE of the DLL.
*/
static mrb_value ClassDLL_to_i(mrb_state* mrb, mrb_value self) {
    return mrb_fixnum_value((mrb_int)DATA_PTR(self));
}

/*[DOCUMENT]
method: HEG::FFI::DLL#close -> self
note: Release the dll
*/
static mrb_value ClassDLL_close(mrb_state* mrb, mrb_value self) {
    HMODULE hDll = (HMODULE)DATA_PTR(self);
    FreeLibrary(hDll);
    DATA_PTR(self) = 0;
    return self;
}


/*[DOCUMENT]
method: HEG::FFI::Pointer::new(addr : Integer) -> ptr :  Pointer
note: Create a pointer to the specified address.
*/
static mrb_value ClassPointer_new(mrb_state* mrb, mrb_value klass) {
    mrb_int addr;
    mrb_get_args(mrb, "i", &addr);
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassPointer, (LPVOID)addr, &ClassPointerDataType));
}

/*[DOCUMENT]
method: HEG::FFI::Pointer::alloc(size : Fixnum) -> ptr : Pointer
note: Alloc a memory sized size and return the Pointer object.
*/
static mrb_value ClassPointer_alloc(mrb_state* mrb, mrb_value klass) {
    mrb_int size;
    mrb_get_args(mrb, "i", &size);
    return mrb_obj_value(mrb_data_object_alloc(mrb, ClassPointer, (LPVOID)GlobalAlloc(GPTR, size), &ClassPointerDataType));
}

/*[DOCUMENT]
method: HEG::FFI::Pointer#read(offset : Fixnum, size : Fixnum) -> data : String
note: Read size bytes from ptr[offset], return packed String. 
*/
static mrb_value ClassPointer_read(mrb_state* mrb, mrb_value self) {
    char* ptr = (char*)DATA_PTR(self);
    mrb_int offset, size;
    mrb_get_args(mrb, "ii", &offset, &size);
    mrb_value res = mrb_str_new_capa(mrb, size);
    char* pres = RSTRING_PTR(res);
    memcpy(pres, ptr, size);
    return res;
}

/*[DOCUMENT]
method: HEG::FFI::Pointer#write(offset_ptr : Fixnum, Fixnum, data : String, offset_data = 0 : Fixnum, size = data.length : Fixnum ) -> self
note: Given 2 or 4 arguments, write data[offset_data, size] to ptr[offset_ptr, size]
*/
static mrb_value ClassPointer_write(mrb_state* mrb, mrb_value self) {
    char* ptr = (char*)DATA_PTR(self);
    mrb_int offset_ptr, offset_data = 0, size, argc;
    mrb_value* argv;
    mrb_get_args(mrb, "*!", &argv, &argc);
    if ((argc != 2) || (argc != 4)) {
        mrb_raise(mrb, mrb->eStandardError_class,
            (std::string("HEG::FFI::Pointer#write: Wrong number of arguments(") + std::to_string(argc) + std::string(" for (2 or 4)")).c_str());
        return self;
    }
    offset_ptr = mrb_fixnum(argv[0]);
    mrb_value data = argv[1];
    size = RSTRING_LEN(data);
    if (argc == 4) {
        offset_data = mrb_fixnum(argv[2]);
        size = mrb_fixnum(argv[3]);
    }    
    memcpy(ptr + offset_ptr, RSTRING_PTR(data) + offset_data, size);
    return self;
}


/*[DOCUMENT]
method: HEG::FFI::Pointer#to_i -> addr : Fixnum
note: Get the value of the pointer.
*/
static mrb_value ClassPointer_to_i(mrb_state* mrb, mrb_value self) {
    return mrb_fixnum_value((mrb_int)(DATA_PTR(self)));
}

/*[DOCUMENT]
method: HEG::FFI::Pointer#free -> self
note: Free the pointer which was created by Pointer::alloc. After this operation, you should not access the data which pointer points.
*/
static mrb_value ClassPointer_free(mrb_state* mrb, mrb_value self) {
    GlobalFree(DATA_PTR(self));
    return self;
}

mrb_data_type ClassFunctionDataType = mrb_data_type{ "Function",
        [](mrb_state* mrb, void* ptr) {
    auto p = (FFIFunction*)ptr; 
    delete p;
} };

/*[DOCUMENT]
method: HEG::FFI::Function::new(addr : Fixnum, type_of_return, types_of_args : Array) -> func : Function
note: Create a new foregin language function object. 
*/
static mrb_value ClassFunction_new(mrb_state* mrb, mrb_value klass) {
    mrb_value func_obj;
    mrb_int addr, ret_t;
    mrb_value arg_t;
    mrb_get_args(mrb, "iiA", &addr, &ret_t, &arg_t);
    std::vector<FFI_CTYPE> atv;
    mrb_int al = RARRAY_LEN(arg_t);
    mrb_value* pat = RARRAY_PTR(arg_t);
    for (int i = 0; i < al; i++) {
        atv.push_back((FFI_CTYPE)mrb_fixnum(pat[i]));
    }
    auto f = new FFIFunction((const void*)addr, (FFI_CTYPE)ret_t, std::move(atv));
    func_obj = mrb_obj_value(mrb_data_object_alloc(mrb,
        ClassFunction, f, &ClassFunctionDataType));
    return func_obj;
}

/*[DOCUMENT]
method: HEG::FFI::Function#call(...) -> retval : Object
note: Call a new foregin language function.
*/
static mrb_value ClassFunction_call(mrb_state* mrb, mrb_value self) {
    auto f = GetNativeObject<FFIFunction>(self);
    mrb_int ac;
    mrb_value* av;
    mrb_get_args(mrb, "*!", &av, &ac);
    if (ac != f->args_type.size()) {
        mrb_raise(mrb, mrb->eStandardError_class, "FFI::Function#call: wrong number of arguments");
        return mrb_nil_value();
    }
    std::vector<FFIFunction::CallValue> cvs;
    cvs.reserve(ac);
    for (int i = 0; i < ac; i++) {
        FFI_CTYPE ct = f->args_type[i];
        FFIFunction::CallValue cv;
        if (ct == FFI_TYPE_INT32 || ct == FFI_TYPE_INT64) {
            cv.as_int64 = mrb_fixnum(av[i]);
        }
        else if (ct == FFI_TYPE_FLOAT || ct == FFI_TYPE_DOUBLE) {
            cv.as_double = mrb_float(av[i]);
        }
        else if (ct == FFI_TYPE_STRING) {
            cv.as_cstr = RSTRING_PTR(av[i]);
        }
        else if (ct == FFI_TYPE_VOIDP) {
            if (av[i].tt == MRB_TT_FIXNUM) {
                cv.as_int64 = mrb_fixnum(av[i]);
            }
            else {
                //TODO: Pointer -
            }
        }
        cvs.push_back(cv);
    }
    auto rv = f->Call((int)ac, cvs.data());
    if (f->return_type == FFI_TYPE_INT32 || f->return_type == FFI_TYPE_INT64) {
        return mrb_fixnum_value(rv.as_int64);
    }
    else if (f->return_type == FFI_TYPE_DOUBLE) {
        return mrb_float_value(mrb, rv.as_double);
    }
    else if (f->return_type == FFI_TYPE_FLOAT) {
        return mrb_float_value(mrb, rv.as_float);
    }
    else if (f->return_type == FFI_TYPE_STRING) {
        return mrb_str_new_cstr(mrb, rv.as_cstr);
    }
    else if (f->return_type == FFI_TYPE_VOIDP) {
        return mrb_fixnum_value(rv.as_int64);
        //TODO: Pointer
    }
    return mrb_nil_value();
}

bool InjectEasyFFIExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* ClassObject = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ModuleFFI = mrb_define_module_under(mrb, HEG, "FFI");
    mrb_define_module_function(mrb, ModuleFFI, "dlopen", FFI_dlopen, MRB_ARGS_REQ(1));
    mrb_define_const(mrb, ModuleFFI, "TYPE_INT32", mrb_fixnum_value(FFI_TYPE_INT32));
    mrb_define_const(mrb, ModuleFFI, "TYPE_INT64", mrb_fixnum_value(FFI_TYPE_INT64));
    mrb_define_const(mrb, ModuleFFI, "TYPE_FLOAT", mrb_fixnum_value(FFI_TYPE_FLOAT));
    mrb_define_const(mrb, ModuleFFI, "TYPE_DOUBLE", mrb_fixnum_value(FFI_TYPE_DOUBLE));
    mrb_define_const(mrb, ModuleFFI, "TYPE_STRING", mrb_fixnum_value(FFI_TYPE_STRING));
    mrb_define_const(mrb, ModuleFFI, "TYPE_VOIDP", mrb_fixnum_value(FFI_TYPE_VOIDP));

    //DLL
    ClassDLL = mrb_define_class_under(mrb, ModuleFFI, "DLL", ClassObject);
    mrb_define_method(mrb, ClassDLL, "to_i", ClassDLL_to_i, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassDLL, "close", ClassDLL_close, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassDLL, "addrof", ClassDLL_addrof, MRB_ARGS_REQ(1));

    //Pointer
    ClassPointer = mrb_define_class_under(mrb, ModuleFFI, "Pointer", ClassObject);
    mrb_define_class_method(mrb, ClassPointer, "new", ClassPointer_new, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, ClassPointer, "alloc", ClassPointer_alloc, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassPointer, "to_i", ClassPointer_to_i, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassPointer, "free", ClassPointer_free, MRB_ARGS_NONE());

    //Function
    ClassFunction = mrb_define_class_under(mrb, ModuleFFI, "Function", ClassObject);
    mrb_define_class_method(mrb, ClassFunction, "new", ClassFunction_new, MRB_ARGS_REQ(3));
    mrb_define_method(mrb, ClassFunction, "call", ClassFunction_call, MRB_ARGS_ANY());
    return true;
}

HFENGINE_NAMESPACE_END
