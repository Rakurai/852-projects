#ifndef __TIME_H
#define __TIME_H

#include <sys/time.h>

#define MILLION 1000000

class Time {
public:
	Time() :
		_time()
	{ }
	Time(const struct timeval& t) :
		_time(t)
	{ }
	Time(int seconds, int microseconds) :
		_time()
	{
		_time.tv_sec = seconds;
		_time.tv_usec = microseconds;
	}
	Time(const Time& t) :
		_time()
	{
		_time.tv_sec = t._time.tv_sec;
		_time.tv_usec = t._time.tv_usec;
	}
	Time& operator=(const Time& t) {
		_time.tv_sec = t.sec();
		_time.tv_usec = t.usec();
		return *this;
	}

	int sec() const { return _time.tv_sec; }
	int usec() const { return _time.tv_usec; }
	float toFloat() const { return sec() + ((float)usec() / 1000000); }
	void mark() { gettimeofday(&_time, 0); }

	bool operator==(const Time& rhs) const {
		return toFloat() - rhs.toFloat() == 0;
	}
	bool operator>(const Time& rhs) const {
		return toFloat() > rhs.toFloat();
	}
	bool operator<(const Time& rhs) const {
		return toFloat() < rhs.toFloat();
	}

	const Time operator-(const Time& rhs) const {
		timeval x = {_time.tv_sec, _time.tv_usec};
		timeval y = {rhs.sec(), rhs.usec()};

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
		Time result(x.tv_sec - y.tv_sec, x.tv_usec - y.tv_usec);
		return result;
	}

private:
	struct timeval _time;
};
/*
void timeval_subtract (struct timeval *result, struct timeval x, struct timeval y) {
	// Perform the carry for the later subtraction by updating y.
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
*/
#endif // __TIME_H
