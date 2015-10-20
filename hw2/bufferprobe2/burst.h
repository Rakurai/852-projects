#ifndef __BURST_H
#define __BURST_H

#include "socket.h"
#include "message.h"

class Burst {
public:
	static const int MAX_LEN = 10000;

	Burst(unsigned short test_type, unsigned int burst_num, unsigned int burst_len, unsigned int payload_len, const char *dest, unsigned short port) :
		size(burst_len),
		buffer_size(Message::minBufferSize(payload_len)),
		buffer(new unsigned char[buffer_size]),
		message(buffer, buffer_size)
	{
		message.header().test_type(test_type);
		message.header().message_type(Message::PROBE);
		message.header().payload_len(payload_len);
		message.header().burst_num(burst_num);
		// seq_num set in sending
		message.header().burst_len(burst_len);

		// fill the payload with a repeating pattern
		for (unsigned int i = 0; i < payload_len; i++)
			message.payload()[i] = '1' + i % 4;

		message.setAddress(dest, port);
	}

	~Burst() {
		delete[] buffer;
	}

	void send(Socket &socket) {
		for (unsigned int seq_num = 0; seq_num < size; seq_num++) {
			message.header().seq_num(seq_num);
			socket.send(message);
		}
	}

private:
	Burst();
	Burst(const Burst&);
	Burst& operator=(const Burst&);

	unsigned int size;
	unsigned int buffer_size;
	unsigned char *buffer;
	Message message;
};

#endif // __BURST_H
