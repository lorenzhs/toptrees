#pragma once

#include <cassert>
#include <random>
#include <vector>
#include <unordered_map>

#include "Common.h"

/// A label interface (pure virtual)
template <typename Value>
struct LabelsT {
	/// Access operator
	/// \param index the index of the label to look up
	/// \returns the label value for the given index
	virtual const Value &operator[](uint index) const = 0;
	/// Set a label
	/// \param id the index of the label to set
	/// \param value the value to set the label to
	virtual void set(uint id, const Value &value) = 0;
};

/// Dummy labels that always return the same value for each index
template <typename Value>
struct FakeLabels : LabelsT<Value> {
	FakeLabels(Value retval) : retval(retval) {}

	/// always return the dummy value on access
	const Value &operator[](uint index) const {
		(void)index;
		return retval;
	};

	/// this does nothing
	void set(uint id, const Value &value) {
		(void)id;
		(void)value;
	};

	Value retval;
};

/// Hashing modulo-wrapping label generator (NOT UNIFORM)
struct IdLabels : LabelsT<int> {
	/// Create a new set of labels
	/// \param modulo the number of different values to assign the labels
	IdLabels(uint modulo) : LabelsT<int>(), modulo(modulo), pointlessInts(modulo) {
		for (uint i = 0; i < modulo; ++i) {
			pointlessInts[i] = i;
		}
	}

	/// Access a label by hashing its index
	const int &operator[](uint index) const {
		// a little bit of hashing
		uint res(0);
		boost_hash_combine(res, index);
		res = res % modulo + modulo; // ensure non-negativity
		return pointlessInts[res % modulo];
	}

	/// this does nothing
	void set(uint id, const int &value) {
		(void)id;
		(void)value;
	}

	uint modulo;
	/// We need this because results need to be returned by reference and are referred to
	/// with pointers elsewhere. As the name says, it's rather pointless, but ah well.
	std::vector<int> pointlessInts;
};

/// Uniformly random labels
template<typename RNG>
struct RandomLabels : LabelsT<int> {
	/// Create a new set of labels, distributed uniformly at random
	/// \param numLabels the number of labels to generate
	/// \param maxLabel the range of labels to generate (e.g., for 0 to 9, specify 10)
	/// \param generator the random generator to use
	RandomLabels(uint numLabels, uint maxLabel, RNG &generator) : LabelsT<int>(), labels(numLabels) {
		std::uniform_int_distribution<int> distribution(0, maxLabel - 1);
		for (uint i = 0; i < numLabels; ++i) {
			labels[i] = distribution(generator);
		}
	}

	const int &operator[](uint index) const {
		assert(index < labels.size());
		return labels[index];
	}

	void set(uint id, const int &value) {
		(void)id;
		(void)value;
	}

	std::vector<int> labels;
};

/// A key-value label storage
/**
 * A double-indexed key-value label storage, allowing efficient access
 * by ID and efficient non-duplicating setting of labels
 *
 * This data structure is based on the following anonymous StackOverflow post:
 * http://stackoverflow.com/a/2562117
 */
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

	uint size() const {
		return values.size();
	}

	uint numKeys() const {
		return keys.size();
	}

	std::vector<int> keys;
	std::vector<const Value *> valueIndex;
	std::unordered_map<Value, int> values;
};
