#ifndef TIMER_H
#define TIMER_H

#include <Windows.h>
#include <stdio.h>

class Timer {

	static double cpu_freq;	// in kHz
	static __int64 counter_start;

public:
	
	static bool init();
	static void start();
	static __int64 get();

	static inline double getSeconds() {	
		return double(Timer::get()-Timer::counter_start)/Timer::cpu_freq;
	}
	static inline double getMilliSeconds() {
		return double(1000*(Timer::getSeconds()));
	}
	static inline double getMicroSeconds() {
		return double(1000000*(Timer::getSeconds()));
	}

};




#endif