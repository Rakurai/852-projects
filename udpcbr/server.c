#include <signal.h>

#include "common.h"
#include "protocol.h"
#include "vector.h"
#include "stats.h"
#include "time.h"
#include <sys/time.h>

int init_sock(unsigned short server_port);
void receive_packets(int sock);

int main(int argc, char *argv[])
{
    unsigned short server_port = 5555;

	/* Create socket for sending/receiving datagrams */
	int sock = init_sock(server_port);

	receive_packets(sock);

	close(sock);

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

void receive_packets(int sock) {
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
				printf("recvfrom failed: %s", strerror(errno));

			continue;
		}

		struct timeval tv;
		gettimeofday(&tv, NULL);
		printf("%d.%d\n", tv.tv_sec, tv.tv_usec);
	}
}

