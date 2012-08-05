#include "timer.h"

double Timer::cpu_freq = 0;
__int64 Timer::counter_start = 0;

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

__int64 Timer::get() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

#elif __linux__

bool Timer::init() { }// stub
void Timer::start() { } // stub
long int Timer::get() { } // stub

#endif