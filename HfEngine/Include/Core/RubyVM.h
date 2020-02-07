#pragma once
#include <ThirdParties.h>
#include <xy_async.h>

HFENGINE_NAMESPACE_BEGIN

class RubyVM : public Utility::ReferredObject {
    struct mrb_state* MRBState;
    struct mrbc_context* MRBLoadContext;
    std::thread* currentThread;
    int lastError;
    std::unordered_map<std::string, RClass*> moduleTable;
public:
    struct mrb_state* GetRuby() const {
        return MRBState;
    }

    void Initialize();
    void Release();

    RubyVM() {
        Initialize();
    }
    ~RubyVM() {
        Release();
    }
    
    void StreamException(mrb_value excep, std::ostream& os);

    //Deal Exception
    //if there is an exception, return true
    //else return false
    bool DealException(); 
    
    void Load(const std::string& filename);

    operator struct mrb_state*() const { return MRBState; }
};

extern thread_local RubyVM* currentRubyVM;


namespace RubyVMManager {
    extern SpinLock lock; 
    extern std::unordered_map<DWORD, HEG::RubyVM*> RubyVM;
    inline void RegisterVM(HEG::RubyVM *vm) {
        lock.lock();
        DWORD tid = GetCurrentThreadId();
        if (RubyVM.find(tid) != RubyVM.end()) {
            lock.unlock();
            //throw std::runtime_error("Duplicated RubyVM on one thread");
            return;
        }
        RubyVM[tid] = vm;
        currentRubyVM = vm;
        lock.unlock();
    }
    inline void UnregisterVM() {
        lock.lock();
        DWORD tid = GetCurrentThreadId();
        auto p = RubyVM.find(tid);
        if (p != RubyVM.end())
            RubyVM.erase(p);
        currentRubyVM = nullptr;
        lock.unlock();
    }
    inline HEG::RubyVM* GetCurrentVM() {
        lock.lock();
        DWORD tid = GetCurrentThreadId();
        auto p = RubyVM.find(tid);
        lock.unlock();
        return p->second; //do not check null
    }
}

HFENGINE_NAMESPACE_END