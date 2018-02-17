#pragma once

#include "referptr.h"
#include "stdlib.h"

namespace Utility{

template<class T>
class HFBuffer : public ReferredObject {
    typedef T *pT;
    pT _ptr;
    int _size;
public:
    
    const pT &ptr = _ptr;
    const int &size = _size;
    HFBuffer() {
        _ptr = nullptr;
    }
    void Initialize(int s) {
        assert(s > 0);
        _size = s;
        _ptr = (pT)malloc(sizeof(T)*size);
    }
    HFBuffer(int s) : HFBuffer() {
        Initialize(s);
    }
    pT Get() {
        return _ptr;
    }
    T& operator[](int index) {
        assert(index < size && index >= 0);
        return _ptr[index];
    }
    ~HFBuffer() {
        free(ptr);
    }
    void Resize(int s) {
        assert(ptr && s > 0);
        _size = s;
        _ptr = (pT)realloc(_ptr, _size);
        
    }
    virtual void Release() {};
};

}