#pragma once

#include <random>
#include <vector>
#include <unordered_map>

#include "Common.h"

template <typename Value>
struct LabelsT {
	virtual const Value &operator[](uint index) const = 0;
	virtual void set(uint id, const Value &value) = 0;
};

template <typename Value>
struct FakeLabels : LabelsT<Value> {
	FakeLabels(Value retval) : retval(retval) {}

	const Value &operator[](uint index) const {
		(void)index;
		return retval;
	};

	void set(uint id, const Value &value) {
		(void)id;
		(void)value;
	};

	Value retval;
};

struct IdLabels : LabelsT<int> {
	IdLabels(uint modulo) : LabelsT<int>(), modulo(modulo), pointlessInts(modulo) {
		for (uint i = 0; i < modulo; ++i) {
			pointlessInts[i] = i;
		}
	}

	const int &operator[](uint index) const {
		// a little bit of hashing
		// TODO: something that makes this uniformly at random
		uint res(0);
		boost_hash_combine(res, index);
		res = res % modulo + modulo; // ensure non-negativity
		return pointlessInts[res % modulo];
	}

	void set(uint id, const int &value) {
		(void)id;
		(void)value;
	}

	uint modulo;
	std::vector<int> pointlessInts;
};

struct RandomLabels : LabelsT<int> {
	RandomLabels(uint numLabels, uint maxLabel, uint seed = 42) : LabelsT<int>(), labels(numLabels) {
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<int> distribution(0, maxLabel - 1);
		for (uint i = 0; i < numLabels; ++i) {
			labels[i] = distribution(generator);
		}
	}

	const int &operator[](uint index) const {
		return labels[index];
	}

	void set(uint id, const int &value) {
		(void)id;
		(void)value;
	}

	std::vector<int> labels;
};

// This data structure is based on the following anonymous StackOverflow post:
// http://stackoverflow.com/a/2562117
template <typename Value>
struct Labels : LabelsT<Value> {
	Labels(int sizeHint = 0) : LabelsT<Value>(), keys(sizeHint), valueIndex(), values() {}

	const Value &operator[](uint index) const {
		return *valueIndex[keys[index]];
	}

	void set(uint id, const Value &value) {
		if (id >= keys.size()) {
			keys.resize(id + 1);
		}

		typename std::unordered_map<Value, int>::iterator it = values.find(value);
		if (it == values.end()) {
			it = values.insert(typename std::unordered_map<Value, int>::value_type(value, (int)values.size())).first;
			valueIndex.push_back(&it->first);
		}

		keys[id] = it->second;
	}

	int size() const {
		return values.size();
	}

	int numKeys() const {
		return keys.size();
	}

	std::vector<int> keys;
	std::vector<const Value *> valueIndex;
	std::unordered_map<Value, int> values;
};
