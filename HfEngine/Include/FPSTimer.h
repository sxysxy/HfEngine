#pragma once
#include "windows.h"
#include <chrono>
#include <thread>

namespace Utility{

template<class cls>
struct FPSTimer {
	
	long long time;
	long long startt;
	double tick;
	int rate;
    int rate_warning;
	void Restart(int r) {
		rate = r;

		long long freq;
		QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
		QueryPerformanceCounter((PLARGE_INTEGER)&startt);
		tick = 1.0 * freq / rate;
		time = 0;

        rate_warning = rate / 10;
	}
	
	void Await() {
		auto ticktime = tick * ++time;
		while (true) {
			long long nowt;
			QueryPerformanceCounter((PLARGE_INTEGER)&nowt);
			long long d = nowt - startt;
            if (d >= ticktime) {
                if(d - ticktime >= rate_warning * tick )Restart(rate); //Skip 
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
        if (ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        else
            std::this_thread::yield();
    }
};
typedef FPSTimer<SleepWait> SleepFPSTimer;

}