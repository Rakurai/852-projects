#ifndef __TEST_H
#define __TEST_H

#include <map>
#include <vector>
#include "time.h"

class ProbeData {
public:
	ProbeData(unsigned int plen) :
		payload_len(plen),
		timestamp()
	{
		gettimeofday(&timestamp, NULL);
	}

	unsigned int payload_len;
	struct timeval timestamp;
};

class BurstData {
public:
	BurstData() :
		_burst_len(),
		_probes()
	{ }

	void addProbe(const Header &header) {
		unsigned int seq_num = header.seq_num();
		unsigned int payload_len = header.payload_len();

		_burst_len = header.burst_len();
		_probes.emplace(seq_num, ProbeData(payload_len));
	}

	unsigned int len() const { return _burst_len; }

	const ProbeData& operator[](int index) const {
		return _probes.at(index);
	}

private:
	unsigned int _burst_len;
	std::map<unsigned int, ProbeData> _probes;
};

class TestData {
public:
	static const int BUFFER_SIZE_TEST = 1;

	static const int MAX_LEN = 10000;
	static const int MAX_DELAY = 1000*1000000;

	TestData(unsigned short test_type, unsigned short num_bursts) :
		_test_type(test_type),
		_bursts(num_bursts, BurstData())
	{ }

	void addProbe(const Header &header) {
		unsigned int burst_num = header.burst_num();
		_bursts[burst_num].addProbe(header);
	}

	unsigned int len() const { return _bursts.size(); }

	const BurstData& operator[](int index) const { return _bursts.at(index); }

private:
	unsigned short _test_type;
	std::vector<BurstData> _bursts;
};

#endif // __TEST_H
