#pragma once
#include "windows.h"
namespace Utility{

template<class cls>
struct FPSTimer {
	
	long long time;
	long long startt;
	double tick;
	int rate;
	void Restart(int r) {
		rate = r;

		long long freq;
		QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
		QueryPerformanceCounter((PLARGE_INTEGER)&startt);
		tick = 1.0 * freq / rate;
		time = 0;
	}
	
	void Await() {
		auto ticktime = tick * ++time;
		while (true) {
			long long nowt;
			QueryPerformanceCounter((PLARGE_INTEGER)&nowt);
			long long d = nowt - startt;
			if (d >= ticktime)break;
			if (d >= 20000)cls::Wait(1);
			else 
				cls::Wait(0);
		}
	}
};

struct SleepWait{
	static void Wait(int s) {
		::Sleep(s);
	}
};
typedef FPSTimer<SleepWait> SleepFPSTimer;

}