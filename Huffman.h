#pragma once

#include <cassert>
#include <iterator>
#include <ostream>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

// This implementation of Huffman codes is based on the C++ implementation from Rosetta code
// The original is licensed under the GNU Free Documentation License 1.2
// It can be found at http://rosettacode.org/wiki/Huffman_coding#C.2B.2B
// (as of 03 August 2014)
// This implementation extends and improves upon it.

class HuffNode {
public:
	const int frequency;
	virtual ~HuffNode() {}
protected:
	HuffNode(const int frequency) : frequency(frequency) {}
};

class HuffLeaf : public HuffNode {
public:
	const int symbolId;
	HuffLeaf(const int symbolId, const int frequency) : HuffNode(frequency), symbolId(symbolId) {}
};

class HuffInnerNode : public HuffNode {
public:
	const int leftId, rightId;
	HuffInnerNode(const int leftId, const int rightId, const int combinedFrequency) :
		HuffNode(combinedFrequency),
		leftId(leftId),
		rightId(rightId) {}
};

template <typename SymbolType>
class HuffmanBuilder {
public:
	typedef std::vector<bool> HuffCode;
	HuffmanBuilder() : numItems(0), symbols(), frequencies(), codes(), nodes() {}

	void addItem(const SymbolType &symbol) {
		auto pos = symbols.find(symbol);
		if (pos == symbols.end()) {
			pos = symbols.insert(typename std::unordered_map<SymbolType, int>::value_type(symbol, (int)symbols.size())).first;
			frequencies.push_back(0);
		}
		frequencies[pos->second]++;
		++numItems;
	}

	template <class InputIterator>
	void addItems(InputIterator begin, InputIterator end) {
		for (auto it = begin; it != end; ++it) {
			addItem(*it);
		}
	}

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

	int getNumSymbols() const {
		return symbols.size();
	}

	int getNumItems() const {
		return numItems;
	}

	HuffCode getCode(const SymbolType &symbol) const {
		return codes[symbols[symbol]];
	}

	int getCodeLength(const SymbolType &symbol) const {
		return codes[symbols[symbol]].size();
	}

	long long getBitsNeeded() const {
		long long bits(0);
		assert(frequencies.size() == codes.size());
		for (uint i = 0; i < frequencies.size(); ++i) {
			bits += frequencies[i] * codes[i].size();
		}
		bits += (symbols.size() - 1) * 2;
		return bits;
	}

	long long getBitsForTableLabels() const {
		return static_cast<long long>(getNumSymbols()) * NUM_DIGITS(getNumSymbols());
	}

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
	void constructTree() {
		std::priority_queue<std::pair<int,int>, std::vector<std::pair<int,int>>, std::greater<std::pair<int,int>>> queue;
		for (uint i = 0; i < frequencies.size(); ++i) {
			assert(frequencies[i] > 0);
			nodes.push_back(new HuffLeaf(i, frequencies[i]));
			queue.push(std::make_pair(frequencies[i], nodes.size() - 1));
		}

		while (queue.size() > 1) {
			auto right = queue.top(); queue.pop();
			auto left = queue.top(); queue.pop();
			const int frequency(right.first + left.first);
			nodes.push_back(new HuffInnerNode(right.second, left.second, frequency));
			queue.push(std::make_pair(frequency, nodes.size() - 1));
		}
	}

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
