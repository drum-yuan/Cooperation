
#include "util.h"

uint64_t get_cur_timestamp()
{
#ifdef WIN32
	LARGE_INTEGER liTime;
	LARGE_INTEGER liFrequency;

	liTime.QuadPart = 0;
	liFrequency.QuadPart = 1;

	::QueryPerformanceCounter(&liTime);
	::QueryPerformanceFrequency(&liFrequency);

	return (liTime.QuadPart * 1000 / liFrequency.QuadPart);
#else
    struct timespec t = {0,0};
    clock_gettime(CLOCK_MONOTONIC, &t);

    return (t.tv_sec * 1000 + t.tv_nsec / 1000000);
#endif
}

void util_sleep(int msecond)
{
#ifdef WIN32
	Sleep(msecond);
#else
	usleep(msecond * 1000);
#endif
}