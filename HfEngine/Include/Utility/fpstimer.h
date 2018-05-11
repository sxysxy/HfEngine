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
            if (d >= ticktime) {
                if(d - ticktime >= 100000 * tick )Restart(rate);
                break;
            }
			if (d >= 20000)cls::Wait(1);
			else 
				cls::Wait(0);
		}
	}

    FPSTimer(int r) {
        Restart(r);
    }
    FPSTimer() {}
};

class SleepWait {
public:
    static void Wait(int ms) {
        typedef void(__stdcall *pSleep)(DWORD);
        static pSleep sleep = (pSleep)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "Sleep");
        sleep(ms);
    }
};
typedef FPSTimer<SleepWait> SleepFPSTimer;

}