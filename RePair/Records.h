#pragma once

#include <limits>
#include <unordered_map>
#include <vector>

namespace RePair {

template <typename DataType>
struct Records {
	Records() : skipSymbol(typename std::numeric_limits<DataType>::max()), text(), prev(), next() {}

	template <typename InputType>
	void init(std::vector<InputType> data) {
		text.push_back(DataType()); // Dummy
		std::copy(data.begin(), data.end(), std::back_inserter(text));

		std::unordered_map<DataType, int> prevOcc;
		next.assign(text.size(), 0);
		prev.assign(text.size(), 0);

		const int maxIndex = (int)text.size() - 1;
		for (int i = 1; i < maxIndex; ++i) {
			// skipSymbol is reserved and must not occur in input
			assert(text[i] != skipSymbol);

			const std::pair<DataType, DataType> pair(text[i], text[i+1]);
			int &previousIndex = prevOcc[pair];
			if (previousIndex != 0) {
				// add into linked list
				next[i] = next[previousIndex];
				next[previousIndex] = i;
			}
			previousIndex = i;
		}

		// fill in the prev pointers
		for (int i = 0; i < next.size(); ++i) {
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

	const DataType skipSymbol;
	std::vector<std::pair<DataType, DataType>> text;
	// previous occurrence index, or, in the case of gaps, index of previous actual symbol
	std::vector<int> prev;
	// index of next occurrence, or, in the case of gaps, index of next actual symbol
	std::vector<int> next;
};

}