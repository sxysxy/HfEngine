#include <Core/Basic.h>
#include <Core/RubyVM.h>
#include <io.h>

HFENGINE_NAMESPACE_BEGIN

thread_local RubyVM* currentRubyVM = nullptr;

namespace RubyVMManager {
    SpinLock lock;
    std::unordered_map<DWORD, HEG::RubyVM*> RubyVM;
}

void RubyVM::Initialize() {
    RubyVMManager::RegisterVM(this);
    MRBState = mrb_open();
}

static void DumpInt(int i, char* s)
{
    char* p = s;
    char* t = s;
    while (i > 0) {
        *p++ = (i % 10) + '0';
        i /= 10;
    }
    if (p == s) *p++ = '0';
    *p = 0;
    p--;  
    while (t < p) {
        char c = *t;
        *t++ = *p;
        *p-- = c;
    }
}
extern "C" {
mrb_value
mrb_require(mrb_state* mrb, mrb_value filename);
}
void RubyVM::Load(const std::string& filename) {
    MRBLoadContext = mrbc_context_new(MRBState);
    MRBLoadContext->capture_errors = true;
    FILE* fp = nullptr;
    /*
    try {
        fopen_s(&fp, filename.c_str(), "r");
        mrb_load_file_cxt(MRBState, fp, MRBLoadContext);
    }
    catch (std::exception & err) {
        mrb_raise(MRBState, MRBState->eStandardError_class, err.what());
    }
    */
    int ai = mrb_gc_arena_save(MRBState);
    mrbc_filename(MRBState, MRBLoadContext, filename.c_str());
    try {
        fopen_s(&fp, filename.c_str(), "r");
        mrb_load_file_cxt(MRBState, fp, MRBLoadContext);
        //mrb_require(MRBState, mrb_str_new_cstr(MRBState, filename.c_str()));
    }
    catch (std::runtime_error& re) {
        mrb_raise(MRBState, MRBState->eStandardError_class, re.what());
    }
    catch (std::exception& exce) {
        mrb_raise(MRBState, MRBState->eStandardError_class, exce.what());
    }
    mrb_gc_arena_restore(MRBState, ai);
    if (fp)
        fclose(fp);
    if (DealException()) {
        
        system("pause");
    }
}

void RubyVM::Release() {
    closed = true;
    mrbc_context_free(MRBState, MRBLoadContext);
    RubyVMManager::UnregisterVM();
    mrb_close(MRBState);
}

void RubyVM::StreamException(mrb_value excep, std::ostream& os) {
    //backtrace:
    struct backtrace_location {
        int32_t lineno;
        mrb_sym method_id;
        const char* filename;
    };
    do {

        
        mrb_value packed = mrb_iv_get(MRBState, excep,
            mrb_intern_lit(MRBState, "backtrace"));
        if (mrb_nil_p(packed)) break;
        if (mrb_array_p(packed)) {

            int n = (int)RARRAY_LEN(packed) - 1;
            if (n == 0) return;

            os << "trace (most recent call last):\n";
            for (int i = 0; i < n; i++) {
                mrb_value entry = RARRAY_PTR(packed)[n - i - 1];
                if (mrb_string_p(entry)) {
                    os << "\t[" << (int)RSTRING_LEN(entry) << "] :" << RSTRING_PTR(entry) << '\n';
                }
            }

            break;
        }

        int ai = mrb_gc_arena_save(MRBState);
        backtrace_location* bt = (backtrace_location*)DATA_PTR(packed);
        if (bt == NULL)
            break;
        int n = (mrb_int)RDATA(packed)->flags;
        {
            int len = 0;
            for (int i = 0; i < n; i++) {
                if (!bt[i].filename && !bt[i].lineno && !bt[i].method_id)
                    continue;
                len++;
        }
        if (len == 0)
            break;
    }
        os << "trace (most recent call last):\n";
        for (int i = 0; i < n; i++) {
            const struct backtrace_location* entry = &bt[n - i - 1];
            if (entry->filename == NULL) continue;
            os << "\t[" << i << "] " << entry->filename << ":" << entry->lineno;
            if (entry->method_id != 0) {
                const char* method_name;
                method_name = mrb_sym_name(MRBState, entry->method_id);
                os << ":in " << method_name;
                mrb_gc_arena_restore(MRBState, ai);
            }
            os << '\n';
        }
    } while (0);
    mrb_value err_message = mrb_funcall(GetRuby(), excep, "inspect", 0);
    char *err = RSTRING_PTR(err_message);
    os << err;
    /*
    if(strstr(err, "SyntaxError")) {
    
        //DWORD r;
        //bool rd = ReadFile(readError, buf, 512, &r, nullptr);
        //if(rd)
        //   os << buf;
        //fscanf(stderr, "%[^\0]", buf);
        //fread(buf, 1, 512, stderr);
        //os << buf;
    }*/
}

bool RubyVM::DealException() {
    if (GetRuby()->exc) {
        mrb_value exc_obj = mrb_obj_value(GetRuby()->exc);
        std::stringstream ss;
        
        StreamException(exc_obj, ss);

        mrb_value excp_msg = mrb_str_new_cstr(MRBState, ss.str().c_str());
        mrb_funcall(GetRuby(), mrb_obj_value(GetRuby()->kernel_module), "show_console", 0);
        mrb_funcall(GetRuby(), mrb_obj_value(GetRuby()->kernel_module), "puts", 1, excp_msg);
        return true;
   
    }
    else {
        return false;
    }
}

HFENGINE_NAMESPACE_END