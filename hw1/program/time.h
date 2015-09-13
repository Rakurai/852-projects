
#include <time.h>

#define MILLION 1000000


void timeval_subtract (struct timeval *result, struct timeval x, struct timeval y) {
	/* Perform the carry for the later subtraction by updating y. */
	if (x.tv_usec < y.tv_usec) {
		int nsec = (y.tv_usec - x.tv_usec) / MILLION + 1;
		y.tv_usec -= MILLION * nsec;
		y.tv_sec += nsec;
	}

	if (x.tv_usec - y.tv_usec > MILLION) {
		int nsec = (x.tv_usec - y.tv_usec) / MILLION;
		y.tv_usec += MILLION * nsec;
		y.tv_sec -= nsec;
	}

	// at this point, x >= y in both respects.
	result->tv_sec = x.tv_sec - y.tv_sec;
	result->tv_usec = x.tv_usec - y.tv_usec;
}
