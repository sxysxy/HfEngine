#include "fpstimer.h"

int main(){
	SleepFPSTimer timer;
	timer.Restart(10);
	timer.Await();
}