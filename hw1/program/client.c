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

void usage() {
	fprintf(stderr, "usage:\n  client <server IP> <port> <num_bursts> <iteration delay> [payload size] [burst_size]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 5)	/* Test for correct number of arguments */
		usage();

	/* get info from parameters , or default to defaults if they're not specified */
    char* server_ip = argv[1];		   /* First arg: server IP address (dotted quad) */
    uint16_t server_port = atoi(argv[2]);
    int num_bursts = atoi(argv[3]);
    int burst_delay = atoi(argv[4]);
    int payload_len = 0;
	int requested_burst_len = -1;

    if (argc >= 6) // allow specifying payload length
        payload_len = atoi(argv[5]);

	if (argc >= 7) // specify burst length to repeat
		requested_burst_len = atoi(argv[6]);

    assert_in_range("server port", server_port, 0, 65535);
    assert_in_range("number of bursts", num_bursts, 1, MAX_NUM_BURSTS);
    assert_in_range("burst iteration delay", burst_delay, 0, MAX_BURST_DELAY);
    assert_in_range("payload length", payload_len, 0, ECHOMAX-48);

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

    int bursts_sent = 0;
    int messages_sent = 0;
	int burst_len = requested_burst_len == -1 ? 100 : requested_burst_len;

    int burst_num;
    for (burst_num = 0; !stopping && burst_num < num_bursts; burst_num++) {
        header->burst_num = htonl(burst_num);

        // determine burst length
		if (requested_burst_len == -1)
        	burst_len *= 2;

        header->burst_len = htonl(burst_len);

        int seq_num;
        for (seq_num = 0; seq_num < burst_len; seq_num++) {

            header->seq_num = htonl(seq_num);

            if (sendto(
                sock,
                message_buffer,
                message_len,
                0,
                (struct sockaddr *) &server_address,
                sizeof(server_address)
            ) != message_len)
        	   DieWithError("sendto() sent a different number of bytes than expected");
        }

        printf("burst %d sent, %d messages\n", burst_num, burst_len);

        messages_sent += burst_len;
        bursts_sent++;
        sleep(burst_delay);
    }

    printf("\ntotal of %d bursts sent, totalling %d messages\n", bursts_sent, messages_sent);

    close(sock);
    exit(0);
}
