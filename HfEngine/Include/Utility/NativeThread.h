#pragma once
#include <thread>
#include <mutex>
#include "referptr.h"

namespace Utility{

template<class DataT>
class NativeThread : public Utility::ReferredObject {
public:
    std::thread native_thread;
    std::mutex native_mutex;
    DataT interact_buffer;
    NativeThread() {}
    NativeThread(int argc, void *argv, const std::function<void(NativeThread *self, int, void*)> &f) {
        Initialize(argc, argv, f);
    }
    void Initialize(int argc, void *argv, const std::function<void(NativeThread *self, int, void*)> &f) {
        native_thread = std::thread([=]() {f(this, argc, argv); });
    }
    void UnInitialize() {
        Join();
    }
    virtual void Release() {UnInitialize(); }
    void Join() {
        if(native_thread.joinable())
            native_thread.join();
    }

    DataT *AccessBuffer(bool access) {
        if (access) {
            native_mutex.lock();
            return &interact_buffer;
        }
        else {
            native_mutex.unlock();
            return nullptr;
        }
    }
};

}