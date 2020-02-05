#pragma once
/*
	Easy Smart Pointer based on reference count.
				--by HfCloud.
*/

#include <assert.h>
#include <atomic>

namespace Utility{


class ReferredObject{
public:
     std::atomic<int> __ref_count;
	
	ReferredObject(){
		__ref_count = 0;
	}
	
	int AddRefer(){
		return ++__ref_count;
	}
	
	int SubRefer(){
        int r = 0;
        if ((r = --__ref_count) <= 0) {
            Release();
            delete this;
        }
        return r;
	}
	
	int GetReferCount() const{
		return __ref_count;
	}
	
	virtual void Release() = 0;
};

template<typename _T>
class ReferPtr {
	typedef _T *_pT;
	typedef _pT *_ppT;

	_pT _ptr;
public:
	ReferPtr() {
		_ptr = nullptr;
	}
	ReferPtr(const _pT _optr) :ReferPtr() {
		(*this) = _optr;
	}
	ReferPtr(const ReferPtr &_optr) :ReferPtr() {
		(*this) = _optr;
	}
    ReferPtr(ReferPtr && _optr) :ReferPtr(){
        _optr.Get()->AddRefer();
        _ptr = _optr.Get(); 
    }
	~ReferPtr() {
		Release();
	}

	_pT Get() const {
		return _ptr;
	}
	_ppT GetAddressOf() const {
		return &_ptr;
	}
	_ppT ReleaseAndGetAddressOf() {
		Release();
		return GetAddressOf();
	}
	_pT operator->() const {
		assert(_ptr);
		
		return _ptr;
	}
    const _T &operator*() const {
        assert(_ptr);

        return (*_ptr);
    }

	void Release() {
		if(!_ptr)return;
        _ptr -> SubRefer();
        _ptr = nullptr;
	}

	explicit operator bool() const {
		return Get() != nullptr;
	}
	_ppT operator&() {
		return ReleaseAndGetAddressOf();
	}

	_pT operator=(const _pT _optr) {
        if ((*this) != _optr) {
            Release();
            _ptr = _optr;
            if(_ptr)
                _ptr->AddRefer();
        }
		return _ptr;
	}
	const ReferPtr &operator=(const ReferPtr &_optr) {
        if ((*this) != _optr) {
            Release();
            _ptr = _optr.Get();
            if(_ptr)
                _ptr->AddRefer();
        }
		return _optr;
	}

	bool operator==(const ReferPtr &_optr) {
		return _ptr == _optr.Get();
	}
	bool operator==(const _pT _optr) {
		return _ptr == _optr;
	}

	bool operator!=(const _pT _optr) {
		return _ptr != _optr;
	}
	bool operator!=(const ReferPtr &_optr) {
		return _ptr != _optr.Get();
	}

	template<typename ... _Arg>
	static ReferPtr New(const _Arg & ..._arg) {
		typedef _T ___T;          //mdzz msvc++ _T __T 
		ReferPtr _ptr = new ___T();
		_ptr->Initialize(_arg...);
		return _ptr;
	}
};

}
