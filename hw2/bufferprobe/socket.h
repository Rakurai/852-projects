#ifndef __SOCKET_H
#define __SOCKET_H

#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <string.h>
#include <stdexcept>

#include "message.h"
#include "signalhandler.h"

class Socket {
public:
	static const int MAX_RETRIES = 9;

	virtual void send(const Message &) const = 0;
	virtual void receive(Message &, unsigned int) const = 0;
	virtual void init() = 0;
};

class UDPSocket : public Socket {
public:
	UDPSocket() :
		_address(),
		_sock(-1),
		_alarm()
	{
		init();
	}

	UDPSocket(unsigned short int port) :
		_address(),
		_sock(-1),
		_alarm()
	{
		init();

		memset(&_address, 0, sizeof(_address));
		_address.sin_family = AF_INET;
		_address.sin_addr.s_addr = htonl(INADDR_ANY);
		_address.sin_port = htons(port);

		if (bind(_sock, (struct sockaddr *) &_address, sizeof(_address)) < 0) {
			close(_sock);
			throw std::runtime_error(std::string("UDPSocket::bind(): ") + strerror(errno));
		}
	}

	~UDPSocket() {
		close(_sock);
	}

	void send(const Message &message) const {
		if (sendto(
			_sock,
			message.bytes(),
			message.size(),
			0,
			(struct sockaddr *) &message.address(),
			sizeof(message.address())
		) != (int)message.size())
			throw std::runtime_error(std::string("UDPSocket::sendto(): ") + strerror(errno));
	}

	// blocking receive
	void receive(Message &message) const {
		receive(message, 0);
	}

	void receive(Message &message, unsigned int timeout) const {

		struct sockaddr_in address;
		unsigned int address_len = sizeof(address);
		memset(&address, 0, address_len);

		if (timeout > 0)
			_alarm.set(timeout);

		int len = recvfrom(
			_sock,
			message.bytes(),
			message.size(),
			0,
			(struct sockaddr *)&address,
			&address_len
		);

		if (timeout > 0)
			_alarm.unset();

		if (len <= 0) {
			if (errno != EINTR)
				throw std::runtime_error(std::string("UDPSocket::recvfrom(): ") + strerror(errno));

			message.size(0);
		}
		else {
			message.size(len);
			message.setAddress(address);
		}
	}

private:
	void init() {
		_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (_sock < 0)
			throw std::runtime_error(std::string("UDPSocket::socket(): ") + strerror(errno));
	}

	struct sockaddr_in _address;
	int _sock;
	AlarmInterrupter _alarm;
};

#endif // __SOCKET_H
