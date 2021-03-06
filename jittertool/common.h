/*********************************************************
*
* Module Name: UDP Echo client/server header file
*
* File Name:    UDPEcho.h
*
* Summary:
*  This file contains common stuff for the client and server
*
* Revisions:
*
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close() */


#define ECHOMAX 65535     /* Longest string to echo */
#define MAX_NUM_BURSTS 10000
#define MAX_BURST_DELAY 1000

#ifndef LINUX
#define INADDR_NONE  0xffffffff
#endif

typedef struct burst_protocol_header_st {
    uint16_t test_type;
    uint16_t message_type;
    uint32_t payload_len;
    uint32_t burst_num;
    uint32_t seq_num;
    uint32_t burst_len;
} burst_protocol_header_t;

void DieWithError(char *errorMessage) {
	fprintf(stderr, "%s: %s\n", errorMessage, strerror(errno));
	exit(1);
}

int stopping = 0;

void cntc() {
    stopping = 1;
}

void catch_alarm(int ignored) {
	// do nothing
}

void assert_in_range(char *field, int value, int min, int max) {
    if (value < min || value > max) {
        fprintf(stderr, "The parameter '%s' must be between %d and %d.\n", field, min, max);
        exit(1);
    }
}
