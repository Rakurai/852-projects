#include <arpa/inet.h>
#include <linux/net_tstamp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

void die(char* s)
{
    perror(s);
    exit(1);
}

int main(int argc, char* argv[])
{
    char* destination_ip = "10.0.0.1";
    int destination_port = 1234;

    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        die("socket()");
    }

    int timestamp_flags = SOF_TIMESTAMPING_TX_SOFTWARE;
    if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPING, &timestamp_flags, sizeof(timestamp_flags)) < 0) {
        die("setsockopt()");
    }

    struct sockaddr_in si_server;
    memset(&si_server, 0, sizeof(si_server));
    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(destination_port);
    if (inet_aton(destination_ip, &si_server.sin_addr) == 0) {
        die("inet_aton()");
    }

    const int buffer_len = 256;
    char buffer[buffer_len];

    const int n_packets = 10;
    for (int i = 0; i < n_packets; ++i) {
        sprintf(buffer, "Packet %d", i);
        if (sendto(sock, buffer, buffer_len, 0, (const sockaddr*) &si_server, sizeof(si_server)) < 0) {
            die("sendto()");
        }

        // Obtain the sent packet timestamp.
        char data[256];
        struct msghdr msg;
        struct iovec entry;
        struct sockaddr_in from_addr;
        struct {
            struct cmsghdr cm;
            char control[512];
        } control;
        int res;

        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = &entry;
        msg.msg_iovlen = 1;
        entry.iov_base = data;
        entry.iov_len = sizeof(data);
        msg.msg_name = (caddr_t)&from_addr;
        msg.msg_namelen = sizeof(from_addr);
        msg.msg_control = &control;
        msg.msg_controllen = sizeof(control);
        if (recvmsg(sock, &msg, MSG_ERRQUEUE) < 0) {
            die("recvmsg()");
        }
    }
    return 0;
}
