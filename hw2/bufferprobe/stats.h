#ifndef __STATS_H
#define __STATS_H

// some statistics stuff

#include <float.h>
#include <math.h>

class Stats {
public:
	Stats() :
		_min(DBL_MAX),
		_max(-DBL_MAX),
		_count(),
		_mean(),
		_m2()
	{ }

	void addSample(double value) {
		_count++;

		double delta = value - _mean;
		_mean += delta / _count;
		_m2 += delta * (value - _mean);

		if (value < _min)
			_min = value;

		if (value > _max)
			_max = value;
	}

	double min() const { return _min; }
	double max() const { return _max; }
	unsigned long count() const { return _count; }
	double mean() const { return _mean; }

	double stddev() const {
		if (_count > 2) {
			double variance = _m2 / (_count-1);
			return sqrt(fabs(variance));
		}

		return 0;
	}

private:
	double _min;
	double _max;
	unsigned long _count;
	double _mean;
	double _m2;
};

#endif // __STATS_H
