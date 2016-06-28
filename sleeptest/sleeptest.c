#include <netdb.h>
#include <signal.h>

#include "common.h"

void usage() {
	fprintf(stderr, "usage:\n  sleeptest <iteration delay>\n");
	exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)	/* Test for correct number of arguments */
		usage();

    int delay = atoi(argv[1]);

    // set up CNT-C catcher
    signal(SIGINT, cntc);

	while (!stopping) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		printf("%d.%06d\n", tv.tv_sec, tv.tv_usec);
		usleep(delay);
	}

    exit(0);
}
