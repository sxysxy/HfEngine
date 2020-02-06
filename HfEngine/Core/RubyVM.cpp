#include <Core.h>

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

void RubyVM::Release() {
    mrb_close(*this);
    RubyVMManager::UnregisterVM();
}

bool RubyVM::DealException() {
    if (GetRuby()->exc) {
        mrb_value exc_obj = mrb_obj_value(GetRuby()->exc);
        mrb_value err_pos = mrb_exc_backtrace(GetRuby(), exc_obj);
        err_pos = mrb_funcall(GetRuby(), err_pos, "inspect", 0);
        mrb_value err_message = mrb_funcall(GetRuby(), exc_obj, "inspect", 0);
        mrb_funcall(GetRuby(), mrb_obj_value(GetRuby()->kernel_module), "show_console", 0);
        mrb_funcall(GetRuby(), mrb_obj_value(GetRuby()->kernel_module), "puts", 2, err_pos, err_message);
        return true;
    }
    else {
        return false;
    }
}

HFENGINE_NAMESPACE_END