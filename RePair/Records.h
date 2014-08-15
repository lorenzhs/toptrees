#pragma once

#include <cassert>
#include <limits>
#include <unordered_map>
#include <vector>

namespace RePair {

// forward declaration
template <typename DataType>
class HashTable;

template <typename DataType>
class Records {
public:
	Records() : symbolCount(0), skipSymbol(std::numeric_limits<DataType>::max()), text(), prev(), next() {}

	template <typename InputType>
	Records(std::vector<InputType> &data) : symbolCount(0), skipSymbol(std::numeric_limits<DataType>::max()), text(), prev(), next() {
		init(data);
	}

	template <typename InputType>
	void init(std::vector<InputType> &data) {
		text.reserve(data.size() + 2);
		text.push_back(skipSymbol); // Dummy for begin

		// add the input symbols
		for (auto it = data.cbegin(); it != data.cend(); ++it) {
			// skipSymbol is reserved and must not occur in input
			assert(*it != skipSymbol);
			text.push_back(static_cast<DataType>(*it));
		}

		text.push_back(skipSymbol); // Dummy for end
		assert(text.size() == data.size() + 2);

		// Create next list
		next.resize(text.size());
		for (uint i = 0; i < next.size(); ++i) {
			next[i] = i;
			++symbolCount;
		}
		symbolCount -= 2; // dummy elements

		prev.assign(text.size(), 0);
		DataType second(text[1]);
		const int maxIndex = (int)text.size() - 2;
		for (int i = 1, nextI; i < maxIndex; i = nextI) {
			DataType first(second);
			nextI = nextIndex(i);
			second = text[nextI];

			assert(first != skipSymbol && second != skipSymbol);

			int hashIndex = findInHash(first, second, prev);
			int prevIndex = prev[hashIndex];
			prev[hashIndex] = i;
			if (prevIndex != 0) {
				// add into linked list
				next[i] = next[prevIndex];
				next[prevIndex] = i;
			}
		}

		// fill in the prev pointers
		prev[0] = 0;
		for (int i = 1; i < (int)next.size(); ++i) {
			if (text[i] == skipSymbol) {
				prev[i] = i - 1;
			} else {
				prev[next[i]] = i;
			}
		}
	}

	int nextIndex(int index) const {
		index += 1;
		if (text[index] == skipSymbol) {
			index = next[index];
		}
		return index;
	}

	int prevIndex(int index) const {
		index -= 1;
		if (text[index] == skipSymbol) {
			index = prev[index];
		}
		return index;
	}

	DataType nextSymbol(const int index) const {
		return text[nextIndex(index)];
	}

	DataType prevSymbol(const int index) const {
		return text[prevSymbol(index)];
	}

	bool occursAt(const int index, const DataType first, const DataType second) const {
		return text[index] == first && nextSymbol(index) == second;
	}

	int nextNonOverlappingOccurrence(int index) {
		const int nextIdx = nextIndex(index);
		index = next[index];
		if (index == nextIdx) {
			index = next[index];
		}
		return index;
	}

	void replacePair(const int index, const DataType newSymbol) {
		const int secondIndex(nextIndex(index));
		const int thirdIndex(nextIndex(secondIndex));

		// weave over the gaps
		prev[thirdIndex - 1] = index;
		next[index + 1] = thirdIndex;

		// replace symbols
		text[index] = newSymbol;
		text[secondIndex] = skipSymbol;
	}

	int remove(const int index) {
		assert(0 <= index && index < (int)prev.size());
		int delta(-1);

		// scan preceding overlapping pairs
		for (int i = index, p = prev[i]; prevIndex(i) == p; i = p, p = prev[p]) {
			delta = -1 - delta;
		}

		if (delta != 0) {
			// scan following overlapping pairs toggling
			for (int i = index, n = next[i]; nextIndex(i) == n; i = n, n = next[n]) {
				delta = -1 - delta;
			}
		}

		// Get the index of the previous occurrence to this one
		int prevIndex = prev[index];
		int nextIndex = next[index];

		next[prevIndex] = nextIndex;
		prev[nextIndex] = prevIndex;

		prev[index] = index;
		next[index] = index;

		return delta;
	}

	// insert pair at index into the DLL so that it precedes nextIndex
	void insertBefore(const int index, const int nextIndex) {
		// nextIndex's old predecessor, will be index's pred. now
		const int prevIndex(prev[nextIndex]);
		prev[index] = prevIndex;
		next[index] = nextIndex;
		next[prevIndex] = index;
		prev[nextIndex] = index;
	}

	int findInHash(const DataType first, const DataType second, std::vector<int> &hash) {
		int previousValue, hashIndex(HashTable<DataType>::hashPair(first, second));
		do {
			hashIndex = (hashIndex + 1) % hash.size();
			previousValue = hash[hashIndex];
		} while (previousValue != 0 && !occursAt(previousValue, first, second));
		return hashIndex;
	}

	void collapse(std::vector<DataType> &out) {
		const int maxIndex = (int) text.size() - 1;
        for (int source = 1; source < maxIndex; source = nextIndex(source)) {
        	if (text[source] != skipSymbol)
        		out.push_back(text[source]);
        }
	}

	int symbolCount;
	const DataType skipSymbol;
	std::vector<DataType> text;
	// previous occurrence index, or, in the case of gaps, index of previous actual symbol
	std::vector<int> prev;
	// index of next occurrence, or, in the case of gaps, index of next actual symbol
	std::vector<int> next;
};

}