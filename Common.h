#pragma once

#include <functional>
#include <random>

// for std::setw
#define NUM_DIGITS(v) ((v >= 1000000000) ? 10 : (v >= 100000000) ? 9 : (v >= 10000000) ? 8 : (v >= 1000000) ? 7 : (v >= 100000) ? 6 : (v >= 10000) ? 5 : (v >= 1000) ? 4 : (v >= 100) ? 3 : (v >= 10) ? 2 : 1)

enum MergeType { NO_MERGE = -1, VERT_WITH_BBN, VERT_NO_BBN, HORZ_LEFT_BBN, HORZ_RIGHT_BBN, HORZ_NO_BBN };


// adapted from: http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
// the magic number is the binary extension of the golden ratio
// the idea is that every bit is random. A more detailed explanation is available at
// http://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
template <typename T>
static void boost_hash_combine(uint &seed, const T &val) {
	std::hash<T> hasher;
	seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// shared random generator
typedef std::mt19937 RandomGeneratorType;
std::mt19937 &getRandomGenerator() {
	static std::mt19937 engine{};
	return engine;
}