#include <iostream>
#include <cstdlib>

#include "burst.h"
#include "parameter.h"
#include "socket.h"
#include "signalhandler.h"
#include "results.h"

void usage() {
	std::cerr << "usage:\n  client <server IP> <port> <num_bursts> <iteration delay> [payload size] [initial burst size] [burst size increase factor]" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
try {
	if (argc < 5)	/* Test for correct number of arguments */
		usage();

	/* get info from parameters , or default to defaults if they're not specified */
	char *server_ip = argv[1];		   /* First arg: server IP address (dotted quad) */
	Parameter<uint16_t>		server_port("server port", 0, 65535, atoi(argv[2]));
	Parameter<int>			num_bursts("number of bursts", 1, TestData::MAX_LEN, atoi(argv[3]));
	Parameter<int>			burst_delay("burst iteration delay", 0, TestData::MAX_DELAY, atoi(argv[4]));
	Parameter<int>			payload_len("payload length", 0, Message::MAX_PAYLOAD, 0);
	Parameter<int>			initial_burst_len("initial burst length", 1, Burst::MAX_LEN, 100);
	Parameter<float>		burst_len_multiplier("burst length multiplier", 0, 100, 0);

	if (argc >= 6) // allow specifying payload length
		payload_len.value(atoi(argv[5]));

	if (argc >= 7) // specify burst length to repeat
		initial_burst_len.value(atoi(argv[6]));

	if (argc >= 8)
		burst_len_multiplier.value(atof(argv[7]));

	// other (currently constant) parameters
	int test_type = TestData::BUFFER_SIZE_TEST;
	int client_port = server_port.value()+1;

	SignalHandler cntlcCatcher(SIGINT);

	// open a socket, bind for control messages
	UDPSocket socket(client_port);

	const unsigned int bufferSize = Message::MAX_SIZE;
	unsigned char controlBuffer[bufferSize];
	unsigned char responseBuffer[bufferSize];

	// first burst length
	int burst_len = initial_burst_len.value();

	Message controlMessage(controlBuffer, 0);
	controlMessage.setAddress(server_ip, server_port.value());

	// send the begin message
	controlMessage.header().message_type(Message::BEGIN);
	controlMessage.header().test_type(test_type);
	controlMessage.header().burst_num(num_bursts.value());
	// seq_num set in sending
	controlMessage.header().burst_len(burst_len);

	bool gotResponse = false;

	for (int i = 0; i < Socket::MAX_RETRIES; i++) {
		controlMessage.header().seq_num(i+1);
		socket.send(controlMessage);

		// wait for the begin_ack
		Message responseMessage(responseBuffer, bufferSize);
		socket.receive(responseMessage, 1);

		if (responseMessage.size() <= 0)
			continue;

		if (!Message::isResponsePair(controlMessage, responseMessage))
			throw std::runtime_error("Got an unexpected response to begin message.");

		gotResponse = true;
		break;
	}

	if (!gotResponse)
		throw std::runtime_error("Didn't get a response to the begin message from the server.");

	int bursts_sent = 0;
	int messages_sent = 0;

	// send the bursts
	for (int burst_num = 0; !cntlcCatcher.receivedSignal() && burst_num < num_bursts.value(); burst_num++) {
		Burst burst(test_type, burst_num, burst_len, payload_len.value(), server_ip, server_port.value());
		burst.send(socket);

		messages_sent += burst_len;
		bursts_sent++;

//		printf("burst %d sent, %d messages\n", burst_num, burst_len);
		burst_len += burst_len * burst_len_multiplier.value();
		usleep(burst_delay.value());
	}

//	printf("\nTotal of %d bursts sent, totalling %d messages\n", bursts_sent, messages_sent);

	// send the complete message
	controlMessage.header().message_type(Message::COMPLETE);
	controlMessage.header().burst_num(num_bursts.value());
	// seq_num set in sending
	controlMessage.header().burst_len(burst_len);

	gotResponse = false;

	for (int i = 0; i < Socket::MAX_RETRIES; i++) {
		controlMessage.header().seq_num(i+1);
		socket.send(controlMessage);

		// wait for the complete_ack
		Message responseMessage(responseBuffer, bufferSize);
		socket.receive(responseMessage, 1);

		if (responseMessage.size() <= 0)
			continue;

		if (!Message::isResponsePair(controlMessage, responseMessage))
			throw std::runtime_error("Got an unexpected response to complete message.");

		TestResults results(responseMessage.payload(), responseMessage.header().payload_len());
		std::cout << results;

		gotResponse = true;
		break;
	}

	if (!gotResponse)
		throw std::runtime_error("Didn't get a response to the complete message from the server.");

} catch (std::exception &e) {
	std::cerr << "exception caught: " << e.what() << std::endl;
}
	exit(0);
}
