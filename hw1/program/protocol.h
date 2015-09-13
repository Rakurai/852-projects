#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "vector.h"
#include <time.h>

typedef struct burst_protocol_header_st {
    uint16_t test_type;
    uint16_t message_type;
    uint32_t payload_len;
    uint32_t burst_num;
    uint32_t seq_num;
    uint32_t burst_len;
} burst_protocol_header_t;

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

#endif // PROTOCOL_H
