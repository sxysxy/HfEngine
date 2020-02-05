#pragma once
#include <ThirdParties.h>

HFENGINE_NAMESPACE_BEGIN

class RubyVM {
    struct mrb_state* MRBState;
    std::thread* currentThread;
    int lastError;
    std::unordered_map<std::string, RClass*> moduleTable;
public:
    struct mrb_state* GetRuby() const {
        return MRBState;
    }

    void injectExtension(const std::function<bool()>);

    operator struct mrb_state*() const { return MRBState; }
};

extern thread_local std::unique_ptr<RubyVM> currentRubyVM;

HFENGINE_NAMESPACE_END