#ifndef __RESULTS_H
#define __RESULTS_H

#include <vector>
#include <iostream>
#include <iomanip>
#include "test.h"
#include "time.h"
#include "stats.h"

class BurstResults {
public:
	BurstResults(const BurstData &burst) :
		num_sent(burst.len()),
		num_received(),
		bytes_received(),
		time_overall(),
		num_contiguous(),
		bytes_contiguous(),
		time_contiguous()
	{
		struct timeval first_timestamp = {0, 0};
		struct timeval last_timestamp;
		bool found_dropped = false;

		for (unsigned int seq_num = 0; seq_num < num_sent; seq_num++) {
			try {
				const ProbeData& probe = burst[seq_num]; // throws out of range

				if (first_timestamp.tv_sec == 0) {
					first_timestamp.tv_sec = probe.timestamp.tv_sec;
					first_timestamp.tv_usec = probe.timestamp.tv_usec;
				}

				last_timestamp.tv_sec = probe.timestamp.tv_sec;
				last_timestamp.tv_usec = probe.timestamp.tv_usec;

				// size of an ethernet frame + ip + udp with this payload
				int packet_len = probe.payload_len + sizeof(Header) + 42;
				bytes_received += packet_len;
				num_received++;

				if (!found_dropped) {
					bytes_contiguous += packet_len;
					num_contiguous++;
				}
			}
			catch (std::out_of_range &e) {
				if (!found_dropped)
					timeval_subtract(&time_contiguous, last_timestamp, first_timestamp);

				found_dropped = true;
				continue;
			}
		}

		timeval_subtract(&time_overall, last_timestamp, first_timestamp);
	}

	float loss() const {
		return (float)(num_sent - num_received)/num_sent;
	}
	float rate() const {
		return (float)bytes_received
		 / (1000000 * time_overall.tv_sec + time_overall.tv_usec);
	}

	unsigned int num_sent;
	unsigned int num_received;
	unsigned int bytes_received;
	struct timeval time_overall;

	unsigned int num_contiguous;
	unsigned int bytes_contiguous;
	struct timeval time_contiguous;
};

class TestResults {
public:
	TestResults(const TestData &test) :
		_burst_results()
	{
		for (unsigned int i = 0; i < test.len(); i++)
			_burst_results.push_back(BurstResults(test[i]));
	}

	//  deserialize a string of bytes
	TestResults(const unsigned char *bytes, unsigned int len) :
		_burst_results()
	{
		unsigned int result_size = sizeof(BurstResults);
		unsigned int num_results = len / result_size;

		for (unsigned int i = 0; i < num_results; i++) {
			unsigned int offset = i * result_size;
			_burst_results.push_back(*(BurstResults *)(bytes + offset));
		}
	}

	unsigned int serialize(unsigned char *bytes) const {
		unsigned char *p = bytes;
		unsigned int len = 0;

		for (std::vector<BurstResults>::const_iterator itr = _burst_results.begin(); itr != _burst_results.end(); itr++) {
			memcpy(p, &*itr, sizeof(BurstResults));
			p += sizeof(BurstResults);
			len += sizeof(BurstResults);
		}

		return len;
	}

	unsigned int len() const { return _burst_results.size(); }

	const BurstResults& operator[](int index) const {
		return _burst_results[index];
	}

private:
	std::vector<BurstResults> _burst_results;
};

std::ostream& operator<<(std::ostream& os, const BurstResults& results) {
	os  << std::setw(5) << results.num_received << "/"
		<< std::setw(5) << results.num_sent << " received ("
		<< std::setw(6) << std::fixed << std::setprecision(2) << results.loss()*100 << "% loss), "
		<< std::setw(8) << results.bytes_received << " bytes total ("
		<< std::setw(6) << std::fixed << std::setprecision(2) << results.rate() << " MB/s), "
		<< std::setw(5) << results.num_contiguous << " messages ("
		<< std::setw(8) << results.bytes_contiguous << " bytes) buffer size";
	return os;
}


std::ostream& operator<<(std::ostream& os, const TestResults& results) {
	Stats bandwidth;
	Stats buffer_size;

	for (unsigned int i = 0; i < results.len(); i++) {
		const BurstResults& burst = results[i];

		if (burst.num_sent > 0) {
			bandwidth.addSample(burst.rate());
			buffer_size.addSample(burst.bytes_contiguous);
		}

		os << "burst " << std::setw(3) << i+1 << ": " << burst << std::endl;
	}

	os << "Sample mean of the bottleneck bandwidth is: " << bandwidth.mean() << " MB/s. (stddev = " << bandwidth.stddev() << ")" << std::endl;
	os << "Sample mean of the bottleneck buffer size is: " << buffer_size.mean() << " bytes. (stddev = " << buffer_size.stddev() << ")" << std::endl;

    return os;
}

#endif // __RESULTS_H
