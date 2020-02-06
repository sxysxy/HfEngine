#include <Core.h>

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
    HANDLE h = (HANDLE)DATA_PTR(self);
    CloseHandle(h);
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
    GlobalFree((DATA_PTR(self)));
    return self;
}

bool InjectEasyFFIExtension() {
    mrb_state* mrb = currentRubyVM->GetRuby();
    RClass* ClassObject = mrb->object_class;
    RClass* HEG = mrb_define_module(mrb, "HEG");
    ModuleFFI = mrb_define_module_under(mrb, HEG, "FFI");
    mrb_define_module_function(mrb, ModuleFFI, "dlopen", FFI_dlopen, MRB_ARGS_REQ(1));
    //DLL
    ClassDLL = mrb_define_class_under(mrb, ModuleFFI, "DLL", ClassObject);
    mrb_define_method(mrb, ClassDLL, "to_i", ClassDLL_to_i, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassDLL, "close", ClassDLL_close, MRB_ARGS_NONE());

    //Pointer
    ClassPointer = mrb_define_class_under(mrb, ModuleFFI, "Pointer", ClassObject);
    mrb_define_class_method(mrb, ClassPointer, "new", ClassPointer_new, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, ClassPointer, "alloc", ClassPointer_alloc, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, ClassPointer, "to_i", ClassPointer_to_i, MRB_ARGS_NONE());
    mrb_define_method(mrb, ClassPointer, "free", ClassPointer_free, MRB_ARGS_NONE());

    //Function
    ClassFunction = mrb_define_class_under(mrb, ModuleFFI, "Function", ClassObject);

    return true;
}

HFENGINE_NAMESPACE_END
