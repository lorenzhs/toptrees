#pragma once

#include <cmath>
#include <ostream>
#include <sstream>
#include <unordered_map>

#include "BinaryDag.h"

/// Calculate entropy of a sequence of symbols
template <typename T, typename CounterType = int>
class EntropyCalculator {
public:
	/// Initialise entropy calculator
	EntropyCalculator() : numItems(0), freq() {}

	/// add an occurence to the entropy calculation
	/// \param item the item to add, by const reference
	void addItem(const T &item) {
		CounterType &count = freq[item];
		++count;
		++numItems;
	}

	/// add an occurence to the entropy calculation
	/// \param item the item to add, by rvalue reference
	void addItem(T &&item) {
		CounterType &count = freq[item];
		++count;
		++numItems;
	}

	/// Add a number of items [begin, end) to the entropy calculation
	/// \param begin iterator to the first item to add
	/// \param end the iterator to the item beyond the last one to add
	template <class InputIterator>
	void addSequence(InputIterator begin, InputIterator end) {
		for (auto it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

	/// The number of distinct symbols encountered
	size_t numSymbols() const {
		return freq.size();
	}

	/// The number of occurences counted
	CounterType numOccurences() const {
		return numItems;
	}

	/// The entropy of a memoryless source using the symbol frequencies observed
	double getEntropy() const {
		double entropy(0.0);
		for (auto it = freq.cbegin(); it != freq.cend(); ++it) {
			if (it->second == 0) continue;
			double relativeFrequency(((double)it->second) / numItems);
			entropy -= relativeFrequency * log2(relativeFrequency);
		}
		return entropy;
	}

	/// The optimal number of bits to code a symbol
	/// \param symbol the symbol by const reference
	double optBitsForSymbol(const T &symbol) const {
		CounterType occurrences(freq.at(symbol));
		if (occurrences == 0) return 0;
		return -1*log2(((double)occurrences) / numItems);
	}

	/// The optimal number of bits to code a symbol
	/// \param symbol the symbol by rvalue reference
	double optBitsForSymbol(T &&symbol) const {
		CounterType occurrences(freq.at(symbol));
		if (occurrences == 0) return 0;
		return -1*log2(((double)occurrences) / numItems);
	}

	/// The rounded-up number of bits to code a symbol, e.g. in a Huffman code
	/// \param symbol the symbol by const reference
	double bitsForSymbol(const T &symbol) const {
		return ceil(optBitsForSymbol(symbol));
	}

	/// The rounded-up number of bits to code a symbol, e.g. in a Huffman code
	/// \param symbol the symbol by rvalue reference
	double bitsForSymbol(T &&symbol) const {
		return ceil(optBitsForSymbol(symbol));
	}

	std::string summary() const {
		std::stringstream s;
		s << "Entropy of " << freq.size() << " symbols with " << numItems << " occurrences: " << getEntropy() << " bits/symbol needed on average" << std::endl;
		return s.str();
	}

	friend std::ostream &operator<<(std::ostream &os, const EntropyCalculator &entropy) {
		os << entropy.summary();
		for (auto it = entropy.freq.cbegin(); it != entropy.freq.cend(); ++it) {
			os << it->first << ": " << entropy.optBitsForSymbol(it->first) << " bits, " << it->second << " occurrences" << std::endl;
		}
		return os;
	}

private:
	CounterType numItems;
	std::unordered_map<T, CounterType> freq;
};

template <typename DataType>
struct DagEntropy {
	DagEntropy(const BinaryDag<DataType> &dag) : entropy(), dag(dag) {}

	double calculate() {
		dag.template inPostOrder<int>([&] (int nodeId, int left, int right) {
			if (left == -1) entropy.addItem(-1);
			if (right == -1) entropy.addItem(-1);
			entropy.addItem(nodeId);
			return 0;
		});
		return entropy.getEntropy();
	}

	EntropyCalculator<int> &getEntropy() {
		return entropy;
	}

private:
	EntropyCalculator<int> entropy;
	const BinaryDag<DataType> &dag;
};
