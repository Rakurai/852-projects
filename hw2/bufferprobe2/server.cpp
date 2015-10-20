#include <sys/time.h>
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "parameter.h"
#include "socket.h"
#include "stats.h"
#include "time.h"
#include "test.h"
#include "results.h"

void usage() {
	std::cerr << "usage:\n  server <port> [test mode] [queue capacity]" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
try {
    if (argc < 2)
        usage();
/*
	test_parameters_t test_parameters;
	test_parameters.test_mode = 0;
	test_parameters.test_queue_len = 100;

	if (argc > 2)
		test_parameters.test_mode = atoi(argv[2]);
	if (argc > 3)
		test_parameters.test_queue_len = atoi(argv[3]);
*/
	Parameter<unsigned short> server_port("server port", 0, 65535, atoi(argv[1]));
	Parameter<unsigned short> test_mode("test mode", 0);
	Parameter<unsigned int> test_queue_len("test queue length", 100);

	if (argc >= 3)
		test_mode.value(atoi(argv[2]));

	if (argc >= 4)
		test_queue_len.value(atoi(argv[3]));

	std::cout << "starting server on port " << server_port.value() << "..." << std::endl;

	/* Create socket for sending/receiving datagrams */
	UDPSocket socket(server_port.value());

	const unsigned int bufferSize = Message::MAX_SIZE;
	unsigned char messageBuffer[bufferSize];

	SignalHandler stopping(SIGINT);

	TestData *testdata;

	while (!stopping.receivedSignal()) {
		Message message(messageBuffer, bufferSize);

		// wait for a message
		socket.receive(message, 1); // max timeout for any message

		if (message.size() <= 0)
			continue;

		switch (message.header().message_type()) {
		case Message::BEGIN:
			if (testdata)
				delete testdata;

			testdata = new TestData(
				message.header().test_type(),
				message.header().burst_num()
			);

			message.header().message_type(Message::BEGIN_ACK);
			socket.send(message);
			break;

		case Message::PROBE:
			// drop packets after simulated queue filled
			if (test_mode.value() == 1
			 && message.header().seq_num() >= test_queue_len.value()) {
				// do nothing
			}
			else
				testdata->addProbe(message.header());

			break;

		case Message::COMPLETE: {
			// process results
			TestResults results(*testdata);

			// create response with results
			unsigned int result_len = results.serialize(message.payload());
			message.header().payload_len(result_len);
			message.header().message_type(Message::COMPLETE_ACK);
			socket.send(message);
			break;
		}

		case Message::ERROR:
			if (testdata) {
				delete testdata;
				testdata = NULL;
			}

			break;
		}
	}

	if (testdata) {
		TestResults results(*testdata);
		std::cout << results;
		delete testdata;
	}
}
catch (std::exception &e) {
	std::cerr << "exception caught: " << e.what() << std::endl;
}

	return 0;
}
