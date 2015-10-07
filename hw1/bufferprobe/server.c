#include <signal.h>

#include "common.h"
#include "protocol.h"
#include "vector.h"
#include "stats.h"
#include "time.h"
#include <sys/time.h>

void usage() {
	fprintf(stderr, "usage:\n  server <port> [test mode] [queue capacity]\n");
	exit(1);
}

int init_sock(unsigned short server_port);
void receive_packets(int sock, vector_t *burst_data_vector);
void analyze_packets(vector_t *burst_data_vector, test_parameters_t *test_parameters, analysis_results_t *analysis_results);
void free_burst_data(vector_t *burst_data_vector);

int main(int argc, char *argv[])
{
    if (argc < 2)
        usage();

    unsigned short server_port = atoi(argv[1]);  /* First arg:  local port */

	test_parameters_t test_parameters;
	test_parameters.test_mode = 0;
	test_parameters.test_queue_len = 100;

	if (argc > 2)
		test_parameters.test_mode = atoi(argv[2]);
	if (argc > 3)
		test_parameters.test_queue_len = atoi(argv[3]);

	assert_in_range("server port", server_port, 0, 65535);

	printf("starting server on port %d...\n", server_port);

	/* Create socket for sending/receiving datagrams */
	int sock = init_sock(server_port);

	vector_t burst_data_vector;
	vector_init(&burst_data_vector, 0);

	receive_packets(sock, &burst_data_vector);

	close(sock);

	analysis_results_t analysis_results;
	analysis_results.total_messages_received = 0;
	analysis_results.total_messages_expected = 0;
	stats_init(&analysis_results.buffer_capacity_stats);
	stats_init(&analysis_results.bottleneck_bandwidth_stats);

	analyze_packets(&burst_data_vector, &test_parameters, &analysis_results);

	free_burst_data(&burst_data_vector);

	int num_bursts = stats_get_count(&analysis_results.buffer_capacity_stats);
	float total_loss_rate = 100 - (100.0 * analysis_results.total_messages_received / analysis_results.total_messages_expected);
	double buffer_capacity_sample_mean = stats_get_mean(&analysis_results.buffer_capacity_stats);
	double bottleneck_bandwidth_sample_mean = stats_get_mean(&analysis_results.bottleneck_bandwidth_stats);

    // print output
    printf("\nbursts detected: %d\n", num_bursts);
    printf("messages received: %d / %d\n", analysis_results.total_messages_received, analysis_results.total_messages_expected);
    printf("overall loss rate: %0.2f%%\n", total_loss_rate);
    printf("buffer capacity sample mean: %0.2f bytes\n", buffer_capacity_sample_mean);
    printf("bottleneck bandwidth sample mean: %0.2f Mb/s\n", bottleneck_bandwidth_sample_mean * 8 / 1000000);

	return 0;
}

int init_sock(unsigned short server_port) {
	int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock < 0)
		DieWithError("socket()");

	/* Construct local address structure */
	struct sockaddr_in server_address; /* Local address */
	memset(&server_address, 0, sizeof(server_address));   /* Zero out structure */
	server_address.sin_family = AF_INET;                /* Internet address family */
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	server_address.sin_port = htons(server_port);      /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		close(sock);
		DieWithError("bind()");
	}

	return sock;
}

void receive_packets(int sock, vector_t *burst_data_vector) {
	// set up CNT-C catcher
    signal(SIGINT, cntc);

	// set up alarm catcher
	struct sigaction myaction;
	myaction.sa_handler = catch_alarm;

    if (sigfillset(&myaction.sa_mask) < 0)
    	DieWithError("sigfillset()");

    myaction.sa_flags = 0;

    if (sigaction(SIGALRM, &myaction, 0) < 0)
    	DieWithError("sigaction()");

	char receive_buffer[ECHOMAX];        /* Buffer for echo string */
	unsigned int burst_num = 0;
	burst_data_t *bd = NULL;

	stopping = 0;
	while (!stopping) {
		struct sockaddr_in client_address; /* Client address */
		unsigned int client_address_len = sizeof(client_address);

		/* Block until receive message from a client, up to 1 second (to check for ctrl-c) */
		alarm(1);

		short int message_len = recvfrom(
			sock,
			receive_buffer,
			ECHOMAX,
			0,
			(struct sockaddr *) &client_address,
			&client_address_len
		);

		if (message_len <= 0) {
			if (errno != EINTR)
				printf("recvfrom failed in burst %d: %s", burst_num, strerror(errno));

			continue;
		}

		burst_protocol_header_t *header = (burst_protocol_header_t *)receive_buffer;
		unsigned int this_burst_num = ntohl(header->burst_num);

		// different than current burst? get it from the vector
		if (this_burst_num != burst_num) {
			burst_num = this_burst_num;
			bd = (burst_data_t *)vector_get(burst_data_vector, burst_num);
		}

		// burst data still null? make a new one
		if (bd == NULL) { // new burst?
			bd = malloc(sizeof(burst_data_t));
			bd->test_type = ntohs(header->test_type);
			bd->burst_len = ntohl(header->burst_len);

			vector_init(&bd->messages, bd->burst_len);
			vector_set(burst_data_vector, burst_num, (void *)bd);
		}

		message_data_t *message_data = malloc(sizeof(message_data_t));
		unsigned int seq_num = ntohl(header->seq_num);
		message_data->message_type = ntohs(header->message_type);
		message_data->payload_len = ntohl(header->payload_len);
		gettimeofday(&message_data->arrival, NULL);

		vector_set(&bd->messages, seq_num, message_data);
	}
}

void analyze_packets(vector_t *burst_data_vector, test_parameters_t *test_parameters, analysis_results_t *analysis_results) {

	for (int burst_num = 0; burst_num < burst_data_vector->size; burst_num++) {
		burst_data_t *bd = (burst_data_t *)vector_get(burst_data_vector, burst_num);

		if (!bd)
			continue;

		unsigned int received = 0;

		for (int message_num = 0; message_num < bd->messages.size; message_num++)
			if (vector_get(&bd->messages, message_num) != NULL)
				received++;

		analysis_results->total_messages_received += received;
		analysis_results->total_messages_expected += bd->burst_len;

		// calculate buffer capacity for this burst, by finding the first dropped packet in the burst
		// as long as we're looping, find the earliest and latest message in the block
		struct timeval *earliest = NULL;
		struct timeval *latest = NULL;
		unsigned int bytes_received = 0, message_num;

		for (message_num = 0; message_num < bd->messages.size; message_num++) {
			// special test mode, simulated queue, disregard messages after queue length
			if (test_parameters->test_mode == 1 && message_num >= test_parameters->test_queue_len)
				break;

			message_data_t *message = (message_data_t *)vector_get(&bd->messages, message_num);

			if (message == NULL)
				break;

			if (earliest == NULL || timercmp(&message->arrival, earliest, <))
				earliest = &message->arrival;

			if (latest == NULL || timercmp(&message->arrival, latest, >))
				latest = &message->arrival;

			// calculate bytes on the wire
			unsigned int message_bytes = sizeof(burst_protocol_header_t) + message->payload_len;
			unsigned short num_frames = message_bytes / 1472 + message_bytes % 1472 ? 1 : 0;
			bytes_received += num_frames * 42 + message_bytes;
		}

		unsigned int buffer_capacity = message_num;
		stats_add(&analysis_results->buffer_capacity_stats, bytes_received);

		// calculate bottleneck bandwidth for this burst
		struct timeval time_diff;
		timeval_subtract(&time_diff, *latest, *earliest);
		double time_float = time_diff.tv_sec + time_diff.tv_usec / 1000000.0;
		double bottleneck_bandwidth = bytes_received / time_float;

		printf("burst %d: %d/%d received, buffer cap %d msgs (%d bytes), bandwidth %f Mb/s\n",
			burst_num, received, bd->burst_len, buffer_capacity, bytes_received, bottleneck_bandwidth*8/1000000);

		stats_add(&analysis_results->bottleneck_bandwidth_stats, bottleneck_bandwidth);
	}
}

void free_burst_data(vector_t *burst_data_vector) {
	for (int burst_num = 0; burst_num < burst_data_vector->size; burst_num++) {
		burst_data_t *bd = (burst_data_t *)vector_get(burst_data_vector, burst_num);

		if (!bd)
			continue;

		for (int message_num = 0; message_num < bd->burst_len; message_num++) {
			message_data_t *md = (message_data_t *)vector_get(&bd->messages, message_num);

			if (md)
				free(md);
		}

		vector_clear(&bd->messages);
		free(bd);
	}

	vector_clear(burst_data_vector);
}
