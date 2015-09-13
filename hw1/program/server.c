/*********************************************************
*
* Module Name: UDP Echo server
*
* File Name:    UDPEchoServer.c
*
* Summary:
*  This file contains the echo server code
*
* Revisions:
*  $A0:  6/15/2008:  don't exit on error
*
*********************************************************/
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

int main(int argc, char *argv[])
{
    if (argc < 2)
        usage();

    unsigned short server_port = atoi(argv[1]);  /* First arg:  local port */
	unsigned short test_mode = 0;
	unsigned short test_queue_len = 100;

	if (argc > 2)
		test_mode = atoi(argv[2]);
	if (argc > 3)
		test_queue_len = atoi(argv[3]);

	assert_in_range("server port", server_port, 0, 65535);

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

	printf("starting server on port %d...\n", server_port);

	/* Create socket for sending/receiving datagrams */
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket()");

	// from here on the socket must be closed!

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

    char receive_buffer[ECHOMAX];        /* Buffer for echo string */
	unsigned int burst_num = 0;
	burst_data_t *bd = NULL;
	vector_t burst_data_vector;
	vector_init(&burst_data_vector, 0);

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
			bd = (burst_data_t *)vector_get(&burst_data_vector, burst_num);
		}

		// burst data still null? make a new one
		if (bd == NULL) { // new burst?
			bd = malloc(sizeof(burst_data_t));
			bd->test_type = ntohs(header->test_type);
			bd->burst_len = ntohl(header->burst_len);

			vector_init(&bd->messages, bd->burst_len);
			vector_set(&burst_data_vector, burst_num, bd);
		}

		message_data_t *message_data = malloc(sizeof(message_data_t));
		unsigned int seq_num = ntohl(header->seq_num);
		message_data->message_type = ntohs(header->message_type);
		message_data->payload_len = ntohl(header->payload_len);
		gettimeofday(&message_data->arrival, NULL);

		vector_set(&bd->messages, seq_num, message_data);
    }

	close(sock);

	// analysis
	int num_bursts = 0;
	int total_messages_received = 0;
	int total_messages_expected = 0;
	stats_t buffer_capacity_stats;
	stats_t bottleneck_bandwidth_stats;

	stats_init(&buffer_capacity_stats);
	stats_init(&bottleneck_bandwidth_stats);

	for (burst_num = 0; burst_num < burst_data_vector.size; burst_num++) {
		bd = (burst_data_t *)vector_get(&burst_data_vector, burst_num);

		if (!bd)
			continue;

		unsigned int received = 0, message_num;

		for (message_num = 0; message_num < bd->messages.size; message_num++)
			if (vector_get(&bd->messages, message_num) != NULL)
				received++;

		total_messages_received += received;
		total_messages_expected += bd->burst_len;

		// calculate buffer capacity for this burst, by finding the first dropped packet in the burst
		// as long as we're looping, find the earliest and latest message in the block
		struct timeval *earliest = NULL;
		struct timeval *latest = NULL;
		unsigned int bytes_received = 0;

		for (message_num = 0; message_num < bd->messages.size; message_num++) {
			// special test mode, simulated queue, disregard messages after queue length
			if (test_mode == 1 && message_num >= test_queue_len)
				break;

			message_data_t *message = (message_data_t *)vector_get(&bd->messages, message_num);

			if (message == NULL)
				break;

			if (earliest == NULL || timercmp(&message->arrival, earliest, <))
				earliest = &message->arrival;

			if (latest == NULL || timercmp(&message->arrival, latest, >))
				latest = &message->arrival;

			// calculate bytes on the wire
			unsigned short num_frames = message->payload_len / 1472 + 1;
			bytes_received += num_frames * 42 + message->payload_len;
		}

		unsigned int buffer_capacity = message_num;
		stats_add(&buffer_capacity_stats, buffer_capacity);

		// calculate bottleneck bandwidth for this burst
		struct timeval time_diff;
		timeval_subtract(&time_diff, *latest, *earliest);
		double time_float = time_diff.tv_sec + time_diff.tv_usec / 1000000.0;
		double bottleneck_bandwidth = bytes_received / time_float;

		printf("burst %d: %d/%d received, buffer cap %d, bandwidth %f B/s\n",
			burst_num, received, bd->burst_len, buffer_capacity, bottleneck_bandwidth);

		stats_add(&bottleneck_bandwidth_stats, bottleneck_bandwidth);

		for (message_num = 0; message_num < bd->burst_len; message_num++) {
			message_data_t *md = (message_data_t *)vector_get(&bd->messages, message_num);

			if (md)
				free(md);
		}

		vector_clear(&bd->messages);
		free(bd);
	}

	vector_clear(&burst_data_vector);

	float total_loss_rate = 100 - (100.0 * total_messages_received / total_messages_expected);
	double buffer_capacity_sample_mean = stats_get_mean(&buffer_capacity_stats);
	double bottleneck_bandwidth_sample_mean = stats_get_mean(&bottleneck_bandwidth_stats);

    // print output
    printf("\nbursts detected: %d\n", num_bursts);
    printf("messages received: %d\n", total_messages_received);
    printf("overall loss rate: %f%%\n", total_loss_rate);
    printf("buffer capacity sample mean: %f\n", buffer_capacity_sample_mean);
    printf("bottleneck bandwidth sample mean: %f\n", bottleneck_bandwidth_sample_mean);

	return 0;
}
