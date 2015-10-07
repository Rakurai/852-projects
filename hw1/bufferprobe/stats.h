#ifndef STATS_H
#define STATS_H

// some statistics stuff

#include <float.h>
#include <math.h>

typedef struct stats_st
{
	double min, max;
	unsigned long count;
	double mean;
	double m2;
} stats_t;

static inline void stats_init(stats_t *s)
{
	s->min = DBL_MAX;
	s->max = -DBL_MAX;
	s->count = 0;
	s->mean = 0.0;
	s->m2 = 0.0;
}

static inline stats_t *stats_new()
{
	stats_t *s = malloc(sizeof(stats_t));
	stats_init(s);
	return s;
}

static inline void stats_delete(stats_t *s) {
	free(s);
}

static inline void stats_add(stats_t *s, double value)
{
	s->count++;
//	sd->sum += value;
//	sd->sum_sq += value * value;
	if (value < s->min) {
		s->min = value;
	}

	if (value > s->max) {
		s->max = value;
	}

	double delta = value - s->mean;
	s->mean += delta / s->count;
	s->m2 += delta * (value - s->mean);
}

static inline unsigned long stats_get_count(stats_t *s) {
	return s->count;
}

static inline double stats_get_mean(stats_t *s) {
	return s->mean;
}

static inline double stats_get_stdev(stats_t *s) {
	if (s->count > 2) {
		double variance = s->m2 / (s->count-1);
		return sqrt(fabs(variance));
	}

	return 0;
}

#endif // STATS_H
