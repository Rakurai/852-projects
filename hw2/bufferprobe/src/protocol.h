#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "stats.h"
#include "messagedata.h"
#include <vector>

typedef struct test_parameters_st {
	unsigned short test_mode;
	unsigned int test_queue_len;
} test_parameters_t;

typedef struct burst_data_st {
	std::vector<MessageData> messages;
	uint16_t test_type;
	uint32_t burst_len;
} burst_data_t;


typedef struct analysis_results_st {
	unsigned int total_messages_received;
	unsigned int total_messages_expected;
	stats_t buffer_capacity_stats;
	stats_t bottleneck_bandwidth_stats;
} analysis_results_t;

#endif // PROTOCOL_H
