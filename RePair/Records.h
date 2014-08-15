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
		text.reserve(data.size() + 1);
		text.push_back(DataType()); // Dummy for begin

		for (auto it = data.cbegin(); it != data.cend(); ++it) {
			text.push_back(static_cast<DataType>(*it));
		}

		text.push_back(DataType()); // Dummy for end

		next.resize(text.size());
		prev.assign(text.size(), 0);

		for (uint i = 0; i < next.size(); ++i) {
			next[i] = i;
			++symbolCount;
			// skipSymbol is reserved and must not occur in input
			assert(text[i] != skipSymbol);
		}
		symbolCount -= 2; // dummy elements

		DataType second(text[1]);
		const int maxIndex = (int)next.size() - 1;
		for (int i = 1, nextI; i < maxIndex; i = nextI) {
			DataType first(second);
			nextI = nextIndex(i);
			second = text[i];

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
		for (int i = 0; i < (int)next.size(); ++i) {
			prev[next[i]] = i;
		}
	}

	int nextIndex(const int index) const {
		int result(index + 1);
		if (text[result] == skipSymbol) {
			result = next[result];
		}
		return result;
	}

	int prevIndex(const int index) const {
		int result(index - 1);
		if (text[result] == skipSymbol) {
			result = prev[result];
		}
		return result;
	}

	DataType nextSymbol(const int index) const {
		return text[nextIndex(index)];
	}

	DataType prevSymbol(const int index) const {
		return text[prevSymbol(index)];
	}

	bool occursAt(const int index, const DataType first, const DataType second) const {
		return text[index] == first && text[index + 1] == second;
	}

	int nextNonOverlappingOccurrence(const int index) {
		int result(next[index]);
		if (result == index + 1) {
			result = next[result];
		}
		return result;
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