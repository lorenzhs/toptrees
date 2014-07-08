#pragma once

#include <vector>
#include <unordered_map>

// This data structure is based on the following anonymous StackOverflow post:
// http://stackoverflow.com/a/2562117
// but extends it to indexing by original id
template<typename Value>
struct Labels {
	Labels(int sizeHint = 0): keys(sizeHint), valueIndex(), values() {}

	const Value &operator[](int index) {
		return *valueIndex[keys[index]];
	}

	void set(int id, const Value &value) {
		if (id >= (int) keys.size()) {
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
	std::vector<const Value*> valueIndex;
	std::unordered_map<Value, int> values;
};
