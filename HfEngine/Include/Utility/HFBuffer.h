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
    void Initialize(int elem_count) {
        assert(elem_count > 0);
        _size = elem_count * sizeof(T);
        _ptr = (pT)malloc(_size);
    }
    void Initialize(T *vptr, int elem_count) {
        assert(vptr);
        _ptr = vptr;
        _size = elem_count * sizeof(T);
    }
    HFBuffer(int elem_count) : HFBuffer() {
        Initialize(elem_count);
    }
    void FroceToNULL() {
        _ptr = nullptr;
        _size = 0;
    }
    pT Get() {
        return _ptr;
    }
    T& operator[](int index) {
        assert(index < size && index >= 0);
        return _ptr[index];
    }
    ~HFBuffer() {
        UnInitialize();
    }
    void UnInitialize() {
        if(_ptr){
            free(ptr);
            _ptr = nullptr;
        }
    }
    virtual void Release() {UnInitialize();};
};

}