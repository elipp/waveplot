#ifndef TIMER_H
#define TIMER_H

namespace Timer {

	double cpu_freq;	// in kHz
	bool init();
	void start();

	__int64 get();


#ifdef _WIN32
	
	__int64 counter_start;


#elif __linux__

	// not yet implemented.

#endif
	
	inline double getSeconds() {	
		return double(Timer::get()-Timer::counter_start)/Timer::cpu_freq;
	}
	inline double getMilliSeconds() {
		return double(1000*(Timer::getSeconds()));
	}	
}

#ifdef _WIN32
bool Timer::init() {
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li)) {
		printf("Timer initialization failed.\n");
		return false;
	}
	cpu_freq = double(li.QuadPart);	// in Hz. this is subject to dynamic frequency scaling, though

	return true;
}

void Timer::start() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	counter_start = li.QuadPart;
}

inline __int64 Timer::get() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

#elif __linux__

bool Timer::init() { }// stub
void Timer::start() { } // stub
long int Timer::get() { } // stub

#endif


#endif