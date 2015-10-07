#ifndef __BURST_H
#define __BURST_H

#include "common.h"
#include "exception.h"

class Burst {
public:
	Burst(unsigned short test_type, unsigned int burst_num, unsigned int burst_len, unsigned int payload_len) :
		size(burst_len),
		message_len(sizeof(burst_protocol_header_t) + payload_len),
		buffer(new unsigned char[message_len]),
		header((burst_protocol_header_t *)buffer)
	{
		memset(buffer, 0, message_len);

		header->test_type = htons(test_type);
		header->message_type = htons(MSG_PROBE);
		header->payload_len = htonl(payload_len);
		header->burst_num = htonl(burst_num);
		// seq_num set in sending
		header->burst_len = htonl(burst_len);
	}

	~Burst() {
		delete[] buffer;
	}

	void send(int sock, struct sockaddr_in &server_address) {
		for (unsigned int seq_num = 0; seq_num < size; seq_num++) {
			header->seq_num = htonl(seq_num);


			if (sendto(
				sock,
				buffer,
				message_len,
				0,
				(struct sockaddr *) &server_address,
				sizeof(server_address)
			) != message_len)
				throw new DieException("sendto() sent a different number of bytes than expected");
		}
	}

private:
	Burst();
	Burst(const Burst&);
	Burst& operator=(const Burst&);

	unsigned int size;
	unsigned int message_len;
	unsigned char *buffer;
	burst_protocol_header_t *header;
};

#endif // __BURST_H
