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
		payload_len(),
		time_overall(),
		num_contiguous(),
		time_contiguous(),
		time_shortest()
	{
		Time first_timestamp;
		Time last_timestamp;
		Time shortest_difference;
		bool found_dropped = false;

		for (unsigned int seq_num = 0; seq_num < num_sent; seq_num++) {
			try {
				const ProbeData& probe = burst[seq_num]; // throws out of range

				if (first_timestamp.sec() == 0) {
					first_timestamp = probe.timestamp;
					payload_len = probe.payload_len;
				}

				if (last_timestamp.sec() != 0) {
					Time difference = probe.timestamp - last_timestamp;
					if (time_shortest.toFloat() == 0 || difference < time_shortest)
						time_shortest = difference;
				}

				last_timestamp = probe.timestamp;

				num_received++;

				if (!found_dropped) {
					num_contiguous++;
				}

			}
			catch (std::out_of_range &e) {
				if (!found_dropped)
					time_contiguous = last_timestamp - first_timestamp;

				found_dropped = true;
				continue;
			}
		}

		time_overall = last_timestamp - first_timestamp;
	}

	float loss() const {
		if (num_sent == 0)
			return 0;
		return (float)(num_sent - num_received)/num_sent;
	}
	int packet_size() const {
		return payload_len + sizeof(Header) + 42;
	}
	float rate() const {
		if (time_overall.toFloat() == 0)
			return 0;
		return (num_received * packet_size())
		 / (1000000 * time_overall.toFloat());
	}
	float peak() const {
		if (time_shortest.toFloat() == 0)
			return 0;
		return packet_size() / (1000000 * time_shortest.toFloat());
	}

	unsigned int num_sent;
	unsigned int num_received;
	unsigned int payload_len;
	Time time_overall;

	unsigned int num_contiguous;
	Time time_contiguous;
	Time time_shortest;
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
		<< std::setw(8) << results.num_received * results.packet_size() << " bytes total ("
		<< std::setw(6) << std::fixed << std::setprecision(2) << results.rate() << " MB/s), "
		<< std::setw(6) << std::fixed << std::setprecision(2) << results.peak() << " MB/s peak), "
		<< "buffer size " << std::setw(5) << results.num_contiguous << " ("
		<< std::setw(8) << results.num_contiguous * results.packet_size() << " bytes)";
	return os;
}


std::ostream& operator<<(std::ostream& os, const TestResults& results) {
	Stats peak;
	Stats bandwidth;
	Stats buffer_size;

	for (unsigned int i = 0; i < results.len(); i++) {
		const BurstResults& burst = results[i];

		if (burst.num_sent > 0) {
			peak.addSample(burst.peak());
			bandwidth.addSample(burst.rate());
			buffer_size.addSample(burst.num_received * burst.packet_size());
		}

		os << "burst " << std::setw(3) << i+1 << ": " << burst << std::endl;
	}

	os << "rate peak:   " << peak << std::endl;
	os << "bandwidth:   " << bandwidth << std::endl;
	os << "buffer size: " << buffer_size << std::endl;

    return os;
}

#endif // __RESULTS_H
