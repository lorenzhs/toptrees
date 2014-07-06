#pragma once

#include <sys/time.h>

class Timer {
public:
	Timer(): start(timestamp()) {}

	void reset() {
		start = timestamp();
	}

	double elapsedMillis() const {
		return timestamp() - start;
	}

	double getAndReset() {
		double t = elapsedMillis();
		reset();
		return t;
	}

private:
	double start;

	static double timestamp() {
		timeval time;
		gettimeofday(&time, nullptr);
		double ms = double(time.tv_usec) / 1000.0;
		return time.tv_sec*1000 + ms;
	}
};
