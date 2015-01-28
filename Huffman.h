#pragma once

#include <cassert>
#include <iterator>
#include <iostream>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "BitWriter.h"

// This implementation of Huffman codes is based on the C++ implementation from Rosetta code
// The original is licensed under the GNU Free Documentation License 1.2
// It can be found at http://rosettacode.org/wiki/Huffman_coding#C.2B.2B
// (as of 03 August 2014)
// This implementation extends and improves upon it.

struct HuffNode {
	const int frequency;
	virtual ~HuffNode() {}
protected:
	HuffNode(const int frequency) : frequency(frequency) {}
};

struct HuffLeaf : public HuffNode {
	const int symbolId;
	HuffLeaf(const int symbolId, const int frequency) : HuffNode(frequency), symbolId(symbolId) {}
};

struct HuffInnerNode : public HuffNode {
	const int leftId, rightId;
	HuffInnerNode(const int leftId, const int rightId, const int combinedFrequency) :
		HuffNode(combinedFrequency),
		leftId(leftId),
		rightId(rightId) {}
};

/// Generic Huffman Code Builder. Only constructs code, does not en-/decode.
template <typename SymbolType>
class HuffmanBuilder {
public:
	typedef std::vector<bool> HuffCode;
	HuffmanBuilder() : numItems(0), symbols(), frequencies(), codes(), nodes() {}

	/// add an occurence to the frequency statistics
	void addItem(const SymbolType &symbol) {
		auto pos = symbols.find(symbol);
		if (pos == symbols.end()) {
			pos = symbols.insert(typename std::unordered_map<SymbolType, int>::value_type(symbol, (int)symbols.size())).first;
			frequencies.push_back(0);
		}
		frequencies[pos->second]++;
		++numItems;
	}

	/// add a sequence of occurences to the frequency statistics
	template <class InputIterator>
	void addItems(InputIterator begin, InputIterator end) {
		for (auto it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

	/// Construct a Huffman code for the symols encountered, and the frequencies with which they were encountered
	void construct() {
		codes.resize(frequencies.size());
		constructTree();
		computeCodes(nodes.size() - 1, HuffCode());

		// Delete the nodes, we don't need them any more
		for (uint i = 0; i < nodes.size(); ++i) {
			assert(nodes[i] != NULL);
			delete nodes[i];
		}
		nodes.clear();
	}

	/// Get the number of different symbols encountered
	int getNumSymbols() const {
		return symbols.size();
	}

	/// Get the total number of occurences encountered
	int getNumItems() const {
		return numItems;
	}

	/// Get the code for a symbol. Must to have called construct() before.
	HuffCode getCode(const SymbolType &symbol) {
		assert(symbols[symbol] < (SymbolType) codes.size());
		return codes[symbols[symbol]];
	}

	/// Get the length of a symbol's code. Need to have called construct() before.
	int getCodeLength(const SymbolType &symbol) const {
		assert(symbols[symbol] < codes.size());
		return codes[symbols[symbol]].size();
	}

	/// Get the number of bits needed to encode the occurrences encountered with the
	/// code that was calculated (need to have called construct() before).
	/// \return size in bits for coding the items and the *structure* of the huffman table
	long long getBitsNeeded() const {
		long long bits(0);
		assert(frequencies.size() == codes.size());
		for (uint i = 0; i < frequencies.size(); ++i) {
			bits += frequencies[i] * codes[i].size();
		}
		bits += (symbols.size() - 1) * 2;
		return bits;
	}

	/// Get the number of bits that are required to store the labels to the table entries,
	/// as fixed-length codes.
	long long getBitsForTableLabels() const {
		return static_cast<long long>(getNumSymbols()) * log2(getNumSymbols());
	}

	/// Get a string representation, including the symbols, their codes and absolute as well as
	/// relative frequencies.
	std::string toString() const {
		std::stringstream os;
		os << "Huffman with " << frequencies.size() << " symbols:" << std::endl;
		for (auto it = symbols.cbegin(); it != symbols.cend(); ++it) {
			os << +it->first << ": ";
			std::copy(codes[it->second].cbegin(), codes[it->second].cend(), std::ostream_iterator<bool>(os));
			os << " (" << codes[it->second].size() << "b)"
			   << " frequency " << frequencies[it->second]
			   << " (" << (frequencies[it->second] * 100.0) / numItems  << "%)"
			   << std::endl;
		}
		return os.str();
	}

	friend std::ostream &operator<<(std::ostream &os, const HuffmanBuilder &huff) {
		return os << "Huffman with " << huff.getNumSymbols() << " symbols and " << huff.getNumItems() << " occurrences, need " << huff.getBitsNeeded() << " bits";
	}

protected:
	/// Construct a Huffman tree from the symbols encountered and the frequencies observed.
	void constructTree() {
		// PQ with operator >. This has the effect of constructing codes that are maximum-variance, but we don't really care.
		std::priority_queue<std::pair<int,int>, std::vector<std::pair<int,int>>, std::greater<std::pair<int,int>>> queue;
		// Fill the queue with all the nodes
		for (uint i = 0; i < frequencies.size(); ++i) {
			assert(frequencies[i] > 0);
			nodes.push_back(new HuffLeaf(i, frequencies[i]));
			queue.push(std::make_pair(frequencies[i], nodes.size() - 1));
		}

		// Process the nodes from the PQ and combine symbols until only one is left
		while (queue.size() > 1) {
			auto right = queue.top(); queue.pop();
			auto left = queue.top(); queue.pop();
			const int frequency(right.first + left.first);
			nodes.push_back(new HuffInnerNode(right.second, left.second, frequency));
			queue.push(std::make_pair(frequency, nodes.size() - 1));
		}
	}

	/// Recursively assign codes to the symbols
	void computeCodes(const int nodeId, const HuffCode &prefix) {
		const HuffNode *node(nodes[nodeId]);
		if (const HuffLeaf *leaf = dynamic_cast<const HuffLeaf*>(node)) {
			codes[leaf->symbolId] = prefix;
		} else if (const HuffInnerNode *innerNode = dynamic_cast<const HuffInnerNode*>(node)) {
			HuffCode leftPrefix = prefix;
			leftPrefix.push_back(false);
			computeCodes(innerNode->leftId, leftPrefix);

			HuffCode rightPrefix = prefix;
			rightPrefix.push_back(true);
			computeCodes(innerNode->rightId, rightPrefix);
		}
	}

	int numItems;
	std::unordered_map<SymbolType, int> symbols;
	std::vector<int> frequencies;
	std::vector<HuffCode> codes;
	std::vector<HuffNode*> nodes;
};

/// construct a blocked huffman coding
/// the size of the output type must be a multiple of the input type's!
template <typename InputType, typename OutputType, int inputSize = sizeof(InputType)*8, int outputSize = sizeof(OutputType)*8>
struct HuffmanBlocker {
	HuffmanBlocker() : blockingFactor(outputSize / inputSize), tempStore(), huffman() {
		assert(outputSize % inputSize == 0);
		tempStore.reserve(blockingFactor);
	}

	/// add an occurence to the frequency statistics
	void addItem(const InputType &symbol) {
		tempStore.push_back(symbol);
		if (tempStore.size() == blockingFactor) {
			flushQueue();
		}
	}

	/// add a sequence of occurences to the frequency statistics
	template <class InputIterator>
	void addItems(InputIterator begin, InputIterator end) {
		for (auto it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

	void flushQueue() {
		const bool verbose = false;

		if (verbose && tempStore.size() < blockingFactor)
			std::cout << "Filling up: have " << tempStore.size() << " need " << blockingFactor << std::endl;
		while (tempStore.size() < blockingFactor) {
			tempStore.push_back(InputType{});
		}

		// move into huffman
		OutputType result{};
		if (verbose) std::cout << "Combining: ";
		for (uint i = 0; i < blockingFactor; ++i) {
			result |= (tempStore[i] << (i * inputSize));
			if (verbose) std::cout << (uint) tempStore[i] << " ";
		}
		if (verbose) std::cout << " => " << (uint)result << std::endl;
		huffman.addItem(result);
		tempStore.clear();
	}

	const uint blockingFactor;
	std::vector<InputType> tempStore;
	HuffmanBuilder<OutputType> huffman;
};

template <typename SymbolType>
class HuffmanWriter {
public:
	HuffmanWriter(HuffmanBuilder<SymbolType> &huffman, BitWriter &writer): huffman(huffman), writer(writer) {}

	void write(const SymbolType &sym) {
		std::cout << "HW: Writing " << (int) sym << std::endl;
		writer.writeBits(huffman.getCode(sym));
	}

	void addItem(const SymbolType &sym) {
		const std::vector<bool> &vec(huffman.getCode(sym));
		buffer.insert(buffer.end(), vec.begin(), vec.end());
	}

	template <typename InputIterator>
	void addItems(InputIterator begin, InputIterator end) {
		for (InputIterator it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

	void writeBuffer() {
		std::cout << "HuffmanWriter: writing buffer of " << buffer.size() << " bits" << std::endl;
		writer.writeBits(buffer);
		buffer.clear();
	}

protected:
	HuffmanBuilder<SymbolType> &huffman;
	BitWriter &writer;
	std::vector<bool> buffer;
};

template <typename InputType, typename OutputType, int inputSize = sizeof(InputType)*8, int outputSize = sizeof(OutputType)*8>
class BlockedHuffmanWriter {
public:
	BlockedHuffmanWriter(HuffmanBuilder<OutputType> &huffman, BitWriter &writer)
		: huffman(huffman)
		, writer(writer)
		, blockingFactor(outputSize / inputSize)
		, tempStore() {
		assert(outputSize % inputSize == 0);
		tempStore.reserve(blockingFactor);
	}

	void addItem(const InputType &symbol) {
		tempStore.push_back(symbol);
		if (tempStore.size() == blockingFactor) {
			flushTempStore();
		}
	}

	template <class InputIterator>
	void addItems(InputIterator begin, InputIterator end) {
		for (auto it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

	void writeBuffer() {
		flushTempStore();
		std::cout << "BlockedHuffmanWriter: writing    " << buffer.size() << " bits" << std::endl;
		writer.writeBits(buffer);
		buffer.clear();
	}


protected:
	void flushTempStore() {
		// move into huffman
		OutputType result{};
		for (uint i = 0; i < blockingFactor; ++i) {
			result |= (tempStore[i] << (i * inputSize));
		}
		const std::vector<bool> &vec(huffman.getCode(result));
		buffer.insert(buffer.end(), vec.begin(), vec.end());
		tempStore.clear();
	}

	HuffmanBuilder<OutputType> &huffman;
	BitWriter &writer;
	const uint blockingFactor;
	std::vector<InputType> tempStore;
	std::vector<bool> buffer;
};
