#ifndef __MESSAGEDATA_H
#define __MESSAGEDATA_H

#include <sys/time.h>

class MessageData {
public:
	MessageData(uint16_t t, uint32_t l, const struct timeval &a) :
		type(t),
		len(l),
		arrival(a)
	{ }

	uint16_t type;
	uint32_t len;
	struct timeval arrival;
};

#endif // __MESSAGEDATA_H
