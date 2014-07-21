#pragma once

#include <chrono>

/// A flexible timer. TimeT is the precision of the timing, while scalingFactor
/// is the factor by which the output will be scaled. The default is to print
/// return milliseconds with microsecond precision.
template<typename TimeT = std::chrono::microseconds, int scalingFactor = 1000, typename ReturnType = double>
struct TimerT {
	TimerT() {
		reset();
	}

	void reset() {
		start = std::chrono::system_clock::now();
	}

	ReturnType get() const {
		static_assert(scalingFactor > 1, "scaling version");
        TimeT duration = std::chrono::duration_cast<TimeT>(std::chrono::system_clock::now() - start);
        return (duration.count() * 1.0) / scalingFactor;
	}

	ReturnType getAndReset() {
		auto t = get();
		reset();
		return t;
	}

private:
	std::chrono::system_clock::time_point start;
};

/// Timer specialisation for a scaling factor of 1 (non-scaling)
template<typename TimeT>
struct TimerT<TimeT, 1, typename TimeT::rep> {
	TimerT() {
		reset();
	}

	void reset() {
		start = std::chrono::system_clock::now();
	}

	typename TimeT::rep get() const {
        TimeT duration = std::chrono::duration_cast<TimeT>(std::chrono::system_clock::now() - start);
        return duration.count();
	}

	typename TimeT::rep getAndReset() {
		auto t = get();
		reset();
		return t;
	}

private:
	std::chrono::system_clock::time_point start;
};

/// A timer that is accurate to microseconds, formatted as milliseconds
typedef TimerT<std::chrono::microseconds, 1000> Timer;
/// A timer that measures milliseconds
typedef TimerT<std::chrono::milliseconds, 1> MilliTimer;