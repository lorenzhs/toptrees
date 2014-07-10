#pragma once

#include <vector>
#include <unordered_map>

template <typename Value>
struct LabelsT {
	LabelsT(Value retval): retval(retval) {}

	virtual const Value &operator[](int index) const {
		(void)index;
		return retval;
	};

	virtual void set(int id, const Value &value) {
		(void)id;
		(void)value;
	};

	Value retval;
};

struct IdLabels : LabelsT<int> {
	IdLabels(int modulo) : LabelsT<int>(0), modulo(modulo), pointlessInts(modulo) {
		for (int i = 0; i < modulo; ++i) {
			pointlessInts[i] = i;
		}
	}

	const int &operator[](int index) const {
		// a little bit of hashing
		index = index + 0x9e3779b9 + (index << 6) + (index >> 2);
		return pointlessInts[index % modulo];
	}

	int modulo;
	std::vector<int> pointlessInts;
};

// This data structure is based on the following anonymous StackOverflow post:
// http://stackoverflow.com/a/2562117
template <typename Value>
struct Labels : LabelsT<Value> {
	Labels(int sizeHint = 0) : LabelsT<Value>(""), keys(sizeHint), valueIndex(), values() {}

	const Value &operator[](int index) const {
		return *valueIndex[keys[index]];
	}

	void set(int id, const Value &value) {
		if (id >= (int)keys.size()) {
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
