#pragma once

#include <sys/time.h>

/// A reasonably precise timing helper that wraps ugly C interfaces
class Timer {
public:
	/// Create and start a new timer
	Timer() : start(timestamp()) {}

	/// reset the timer
	void reset() {
		start = timestamp();
	}

	/// milliseconds ellapsed since last reset or instantiation
	double elapsedMillis() const {
		return timestamp() - start;
	}

	/// get the elapsed milliseconds and reset the timer
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
		return time.tv_sec * 1000 + ms;
	}
};
