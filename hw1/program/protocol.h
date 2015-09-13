#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "vector.h"
#include "stats.h"
#include <sys/time.h>

typedef struct test_parameters_st {
	unsigned short test_mode;
	unsigned int test_queue_len;
} test_parameters_t;

typedef struct burst_data_st {
	vector_t messages;
	uint16_t test_type;
	uint32_t burst_len;
} burst_data_t;

typedef struct message_data_st {
	uint16_t message_type;
	uint32_t payload_len;
	struct timeval arrival;
} message_data_t;

typedef struct analysis_results_st {
	unsigned int total_messages_received;
	unsigned int total_messages_expected;
	stats_t buffer_capacity_stats;
	stats_t bottleneck_bandwidth_stats;
} analysis_results_t;

#endif // PROTOCOL_H
