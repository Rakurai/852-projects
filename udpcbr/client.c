#include <netdb.h>
#include <signal.h>

#include "common.h"
#include "protocol.h"

void usage() {
	fprintf(stderr, "usage:\n  client <server_ip> <iteration delay> [payload size]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 3)	/* Test for correct number of arguments */
		usage();

	/* get info from parameters , or default to defaults if they're not specified */
    char* server_ip = argv[1];		   /* First arg: server IP address (dotted quad) */
    uint16_t server_port = 5555;
    int delay = atoi(argv[2]);
    int payload_len = 0;

    if (argc >= 4) // allow specifying payload length
        payload_len = atoi(argv[3]);

    // set up CNT-C catcher
    signal(SIGINT, cntc);

	/* Construct the server address structure */
	struct sockaddr_in server_address; /* Echo server address */
	memset(&server_address, 0, sizeof(server_address));	/* Zero out structure */
	server_address.sin_family = AF_INET;				 /* Internet addr family */
	server_address.sin_addr.s_addr = inet_addr(server_ip);  /* Server IP address */

	/* If user gave a dotted decimal address, we need to resolve it  */
	if (server_address.sin_addr.s_addr == -1) {
		struct hostent *thehost = gethostbyname(server_ip);
		server_address.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
	}

	server_address.sin_port = htons(server_port);

	/* Create a datagram/UDP socket */
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

    // set up message bytes with header pointing at beginning 20
    char message_buffer[ECHOMAX];
    int message_len = sizeof(burst_protocol_header_t) + payload_len;
    memset(&message_buffer, 0, sizeof(message_buffer));

    burst_protocol_header_t *header = (burst_protocol_header_t *)message_buffer;
    header->test_type = htons(1);
    header->message_type = htons(1);
    header->payload_len = htonl(payload_len);

	struct timeval start;
	struct timeval stop, diff;
	gettimeofday(&start, NULL);

	int seq_num = 0;
	while (!stopping) {
		header->seq_num = htonl(++seq_num);

		if (sendto(
			sock,
			message_buffer,
			message_len,
			0,
			(struct sockaddr *) &server_address,
			sizeof(server_address)
		) != message_len)
			DieWithError("sendto() sent a different number of bytes than expected");
/*
			gettimeofday(&stop, NULL);
			timeval_subtract(&diff, &stop, &start);
			printf("%d, %d", diff.tv_sec, diff.tv_usec);
	        	gettimeofday(&start, NULL);
			int us = diff.tv_sec * 1000000 + diff.tv_usec;
			printf("has been %d microseconds", us);
*/
		usleep(delay);
//	        	usleep(delay - (us - delay));

	}

    printf("%d packets sent", seq_num);
    close(sock);
    exit(0);
}
