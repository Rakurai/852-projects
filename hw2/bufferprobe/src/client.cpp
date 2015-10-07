/*********************************************************
* Module Name: UDP Echo client source
*
* File Name:	UDPEchoClient2.c
*
* Summary:
*  This file contains the echo Client code.
*
* Revisions:
*
* $A0: 6-15-2008: misc. improvements
*
*********************************************************/
#include <netdb.h>
#include <signal.h>

#include "common.h"
#include "protocol.h"
#include "burst.h"
#include "parameter.h"

void usage() {
	fprintf(stderr, "usage:\n  client <server IP> <port> <num_bursts> <iteration delay> [payload size] [burst_size]\n");
	exit(1);
}

int TEST_TYPE = 1;

int main(int argc, char *argv[])
{
	if (argc < 5)	/* Test for correct number of arguments */
		usage();

	/* get info from parameters , or default to defaults if they're not specified */
	char *server_ip = argv[1];		   /* First arg: server IP address (dotted quad) */
	Parameter<uint16_t>		server_port("server port", 0, 65535, atoi(argv[2]));
	Parameter<int>			num_bursts("number of bursts", 1, MAX_NUM_BURSTS, atoi(argv[3]));
	Parameter<int>			burst_delay("burst iteration delay", 0, MAX_BURST_DELAY, atoi(argv[4]));
	Parameter<int>			payload_len("payload length", 0, ECHOMAX-48, 0);
	Parameter<int>			requested_burst_len("burst length", -1, -1, MAX_BURST_LEN);

	if (argc >= 6) // allow specifying payload length
		payload_len.value(atoi(argv[5]));

	if (argc >= 7) // specify burst length to repeat
		requested_burst_len.value(atoi(argv[6]));

	// set up CNT-C catcher
	signal(SIGINT, cntc);

	/* Construct the server address structure */
	struct sockaddr_in server_address; /* Echo server address */
	memset(&server_address, 0, sizeof(server_address));	/* Zero out structure */
	server_address.sin_family = AF_INET;				 /* Internet addr family */

	int address = inet_addr(server_ip);

	/* If user gave a dotted decimal address, we need to resolve it  */
	if (address == -1) {
		struct hostent *thehost = gethostbyname(server_ip);
		address = *((unsigned long *) thehost->h_addr_list[0]);
	}

	server_address.sin_addr.s_addr = address;  /* Server IP address */
	server_address.sin_port = htons(server_port.value());

	/* Create a datagram/UDP socket */
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		throw DieException("socket() failed");

	// set up message bytes with header pointing at beginning 20
	char message_buffer[ECHOMAX];
//	int message_len = sizeof(burst_protocol_header_t) + payload_len.value();
	memset(&message_buffer, 0, sizeof(message_buffer));

	burst_protocol_header_t *header = (burst_protocol_header_t *)message_buffer;
	header->test_type = htons(1);
	header->message_type = htons(MSG_PROBE);
	header->payload_len = htonl(payload_len.value());

	int bursts_sent = 0;
	int messages_sent = 0;
	int burst_len = requested_burst_len.value() == -1 ? 100 : requested_burst_len.value();

	// send the begin message

	// wait for the begin_ack

	int burst_num;
	for (burst_num = 0; !stopping && burst_num < num_bursts.value(); burst_num++) {
		header->burst_num = htonl(burst_num);

		Burst burst(TEST_TYPE, burst_num, burst_len, payload_len.value());
		burst.send(sock, server_address);

		messages_sent += burst_len;
		bursts_sent++;

		printf("burst %d sent, %d messages\n", burst_num, burst_len);
		sleep(burst_delay.value());
	}

	// send the complete message

	// wait for complete_ack

	printf("\ntotal of %d bursts sent, totalling %d messages\n", bursts_sent, messages_sent);

	close(sock);
	exit(0);
}
