#pragma once

#include <windows.h>
#include <utility>

template<class T>
struct SwapData {
	T a, b;
	T *volatile pa;
    T *volatile pb;
	inline SwapData() {
		pa = std::addressof(a);
		pb = std::addressof(b);
	}
	inline void Swap() {
		pa = (T *)InterlockedExchangePointer((PVOID volatile *)&pb, pa);
	}
	inline T *WritePtr() {
		return pa;
	}
	inline T *ReadPtr() {
		return pb;
	}
	inline T &WriteRef() {
		return (*pa);
	}
	inline T &ReadRef() {
		return (*pb);
	}
};
