#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <netdb.h>
#include <stdexcept>

class Header {
public:
	void test_type(unsigned short t) { _test_type = htons(t); }
	unsigned short test_type() const { return ntohs(_test_type); }
	void message_type(unsigned short t) { _message_type = htons(t); }
	unsigned short message_type() const { return ntohs(_message_type); }
	void payload_len(unsigned int t) { _payload_len = htonl(t); }
	unsigned int payload_len() const { return ntohl(_payload_len); }
	void burst_num(unsigned int t) { _burst_num = htonl(t); }
	unsigned int burst_num() const { return ntohl(_burst_num); }
	void seq_num(unsigned int t) { _seq_num = htonl(t); }
	unsigned int seq_num() const { return ntohl(_seq_num); }
	void burst_len(unsigned int t) { _burst_len = htonl(t); }
	unsigned int burst_len() const { return ntohl(_burst_len); }

private:
	unsigned short _test_type;
	unsigned short _message_type;
	unsigned int _payload_len;
	unsigned int _burst_num;
	unsigned int _seq_num;
	unsigned int _burst_len;
};

// IMPORTANT: Message is a wrapper around a byte array.  It does not own
// the pointer, and cannot be instantiated without one.

class Message {
public:
	static const unsigned int MAX_PAYLOAD = 65500;
	static const unsigned int MAX_SIZE = MAX_PAYLOAD + sizeof(Header);

	enum type {
		PROBE = 1,
		BEGIN,
		BEGIN_ACK,
		COMPLETE,
		COMPLETE_ACK,
		ERROR = 8
	};

	// modifiers
	void setAddress(const struct sockaddr_in& a) { _address = a; }
	void setAddress(const char *dest, unsigned short port) {
		int temp_address = inet_addr(dest);

		/* If user gave a dotted decimal address, we need to resolve it  */
		if (temp_address == -1) {
			struct hostent *thehost = gethostbyname(dest);
			temp_address = *((unsigned long *) thehost->h_addr_list[0]);
		}

		_address.sin_addr.s_addr = temp_address;
		_address.sin_port = htons(port);
	}


	// accessors
	const Header &header() const { return *_header; }
	Header &header() { return *_header; }

	unsigned int size() const { return sizeof(Header) + _header->payload_len(); }
	void size(unsigned int s) { _header->payload_len(s - sizeof(Header)); }

	const struct sockaddr_in& address() const { return _address; }

	unsigned char * payload() { return _payload; }
	const unsigned char * payload() const { return _payload; }

	unsigned char * bytes() { return _bytes; }
	const unsigned char * bytes() const { return _bytes; }

	static unsigned int minBufferSize(unsigned int payload_size) {
		return payload_size + sizeof(Header);
	}

	static bool isResponsePair(Message &control, Message &response) {
		if (control.header().message_type() == Message::BEGIN) {
			if (response.header().message_type() != Message::BEGIN_ACK)
				return false;
		}
		else if (control.header().message_type() == Message::COMPLETE) {
			if (response.header().message_type() != Message::COMPLETE_ACK)
				return false;
		}
		else
			return false;

		if (control.header().burst_num() != response.header().burst_num()
		 || control.header().seq_num() != response.header().seq_num()
		 || control.header().burst_len() != response.header().burst_len())
			return false;

		return true;
	}

	Message(unsigned char *bytes, unsigned int payload_len) :
		_bytes(bytes),
		_header((Header *)_bytes),
		_payload(_bytes + sizeof(Header)),
		_address()
	{
		if (bytes == NULL)
			throw std::runtime_error("cannot instantiate Message with NULL data");

		_header->payload_len(payload_len);
	}

private:
	Message();
	Message(const Message &);
	Message* operator=(const Message&);

	unsigned char *_bytes;
	Header *_header;
	unsigned char *_payload;
	struct sockaddr_in _address;
};
/*
class ControlMessage : public Message {
public:
	ControlMessage(const Message &m) :
		Message(m) { }
protected:
	ControlMessage(unsigned char *bytes, unsigned int size) :
		Message(bytes, size) { }
};

class ResponseMessage : public Message {
public:
	ResponseMessage(const Message &m) :
		Message(m) { }
protected:
	ResponseMessage(unsigned char *bytes, unsigned int size) :
		Message(bytes, size) { }
};

class ProbeMessage : public Message {
	ProbeMessage(unsigned char *bytes, unsigned int size) :
		Message(bytes, size)
	{
		header().message_type(Message::PROBE);
	}

	friend class MessageFactory;
};

class BeginMessage : public ControlMessage {
	BeginMessage(unsigned char *bytes, unsigned int size) :
		ControlMessage(bytes, size)
	{
		header().message_type(Message::BEGIN);
	}

public:
	BeginMessage(const Message &m) :
		ControlMessage(m) { }

	friend class MessageFactory;
};

class BeginAckMessage : public ResponseMessage {
	BeginAckMessage(unsigned char *bytes, unsigned int size) :
		ResponseMessage(bytes, size)
	{
		header().message_type(Message::BEGIN_ACK);
	}
public:
	BeginAckMessage(const Message &m) :
		ResponseMessage(m) { }

	bool isResponseTo(BeginMessage &control) const {
		if (control.header().burst_num() != header().burst_num()
		 || control.header().seq_num() != header().seq_num()
		 || control.header().burst_len() != header().burst_len())
			return false;

		return true;
	}

	friend class MessageFactory;
};

class CompleteMessage : public ControlMessage {
	CompleteMessage(unsigned char *bytes, unsigned int size) :
		ControlMessage(bytes, size)
	{
		header().message_type(Message::COMPLETE);
	}

public:
	CompleteMessage(const Message &m) :
		ControlMessage(m) { }

	friend class MessageFactory;
};

class CompleteAckMessage : public ResponseMessage {
	CompleteAckMessage(unsigned char *bytes, unsigned int size) :
		ResponseMessage(bytes, size)
	{
		header().message_type(Message::COMPLETE_ACK);
	}

public:
	CompleteAckMessage(const Message &m) :
		ResponseMessage(m) { }

	bool isResponseTo(CompleteMessage &control) const {
		if (control.header().burst_num() != header().burst_num()
		 || control.header().seq_num() != header().seq_num()
		 || control.header().burst_len() != header().burst_len())
			return false;

		return true;
	}

	friend class MessageFactory;
};

class ErrorMessage : public Message {
	ErrorMessage(unsigned char *bytes, unsigned int size) :
		Message(bytes, size)
	{
		header().message_type(Message::ERROR);
	}

	friend class MessageFactory;
};

class MessageFactory {
public:
	static Message newMessage(unsigned char *bytes, unsigned int size) {
		Header *header = (Header *)bytes;
		return newMessage(header->message_type(), bytes, size);
	}

	static Message newMessage(unsigned short type, unsigned char *bytes, unsigned int size) {
		switch (type) {
			case Message::PROBE:	return ProbeMessage(bytes, size);
			case Message::BEGIN:	return BeginMessage(bytes, size);
			case Message::BEGIN_ACK:	return BeginAckMessage(bytes, size);
			case Message::COMPLETE:		return CompleteMessage(bytes, size);
			case Message::COMPLETE_ACK:	return CompleteAckMessage(bytes, size);
			case Message::ERROR:		return ErrorMessage(bytes, size);
			default:
				throw DieException("unknown message type");
		}
	}

private:
	MessageFactory();
	MessageFactory(const MessageFactory &);
	MessageFactory* operator=(const MessageFactory &);
};
*/
#endif // __MESSAGE_H
