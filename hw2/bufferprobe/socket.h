#ifndef __SOCKET_H
#define __SOCKET_H

#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <string.h>
#include <stdexcept>
#include <linux/net_tstamp.h>

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

protected:
	struct sockaddr_in _address;
	int _sock;
	AlarmInterrupter _alarm;

	virtual void init() {
		_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (_sock < 0)
			throw std::runtime_error(std::string("UDPSocket::socket(): ") + strerror(errno));
	}
};

class TimestampingUDPSocket : public UDPSocket {
public:
	TimestampingUDPSocket(unsigned short int port) :
		UDPSocket(port)
	{
		init();
	}


	void send(const Message &message) const {
		UDPSocket::send(message);

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

		fd_set readset;
		int result;
		do {
			FD_ZERO(&readset);
			FD_SET(_sock, &readset);
			result = select(_sock + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);

		if (result > 0) {
			if (FD_ISSET(_sock, &readset)) {
		        if (recvmsg(_sock, &msg, MSG_ERRQUEUE) < 0)
					throw std::runtime_error(std::string("TimestampingUDPSocket::recvmsg(): ") + strerror(errno));
std::cout << "here" << std::endl;

	        }
	    }
		else
			throw std::runtime_error(std::string("TimestampingUDPSocket::select(): ") + strerror(errno));
	}

private:
	void init() {
		int timestamp_flags = SOF_TIMESTAMPING_TX_SOFTWARE;
		if (setsockopt(_sock, SOL_SOCKET, SO_TIMESTAMPING, &timestamp_flags, sizeof(timestamp_flags)) < 0)
			throw std::runtime_error(std::string("TimestampingUDPSocket::setsockopt(): ") + strerror(errno));
	}
};

#endif // __SOCKET_H
