#pragma once

#include <cmath>
#include <ostream>
#include <sstream>
#include <unordered_map>

#include "BinaryDag.h"
#include "Labels.h"

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

	/// Bits per symbol if each symbol is coded with an integral number of bits (e.g., Huffman)
	double getIntEntropy() const {
		double entropy(0);
		for (auto it = freq.cbegin(); it != freq.cend(); ++it) {
			if (it->second == 0) continue;
			double relativeFrequency(((double)it->second) / numItems);
			entropy += relativeFrequency * ceil(-1*log2(relativeFrequency));
		}
		return entropy;
	}

	/// Total bits needed for a huffman code
	int huffBitsNeeded() const {
		int codedBits = (int)(getIntEntropy() * numItems);
		int huffTableBits = 2 * (freq.size() - 1);
		return codedBits + huffTableBits;
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

	/// A short string summary of the data collected
	std::string summary() const {
		std::stringstream s;
		s << freq.size() << " symbols, " << numItems << " occurrences: "
		  << getEntropy()  << " / " << getIntEntropy() << " b/symbol on avg; "
		  << " Huff table: " << 2 * (freq.size() - 1) << "b, data " << (int)(getIntEntropy() * numItems) << "b" << std::endl;
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

enum NodeEncoding { LEAF = 0, IMPLICIT_SIBLING = 1, POINTER_SIBLING = 2 };

/// Calculate the different entropies of a BinaryDAG - its structure, its merge types, and its labels
template <typename DataType>
struct DagEntropy {
	DagEntropy(const BinaryDag<DataType> &dag) : dagStructureEntropy(), dagPointerEntropy(), labelEntropy(), mergeEntropy(), dag(dag) {}

	/// Do the entropy calculations on the DAG's nodes
	void calculate() {
		vector<bool> alreadyVisited(dag.nodes.size(), false);

		const auto processNode([&](const int nodeId) {
			if (nodeId < 0) return;
			assert((dag.nodes[nodeId].left < 0) == (dag.nodes[nodeId].right < 0));
			if (dag.nodes[nodeId].left < 0) {
				dagStructureEntropy.addItem(LEAF);
			} else {
				if (alreadyVisited[nodeId]) {
					dagStructureEntropy.addItem(POINTER_SIBLING);
					dagPointerEntropy.addItem(nodeId);
				} else {
					dagStructureEntropy.addItem(IMPLICIT_SIBLING);
					alreadyVisited[nodeId] = true;
				}
			}
		});

		// nodeId starts at 1 because 0 is a dummy node, we don't need to code it
		for (uint nodeId = 1; nodeId < dag.nodes.size(); ++nodeId) {
			// DAG node is coded as the IDs of its children, its own ID
			// is implicit from the position in the output it appears in

			const DagNode<DataType> &node(dag.nodes[nodeId]);
			processNode(node.left);
			processNode(node.right);

			if (node.left >= 0 || node.right >= 0) {
				assert(node.left >= 0 && node.right >= 0);
				assert(node.label == NULL);
				assert(node.mergeType != NO_MERGE);
				mergeEntropy.addItem((char)dag.nodes[nodeId].mergeType);
			} else {
				assert(node.left < 0 && node.right < 0);
				assert(node.label != NULL);
				assert(node.mergeType == NO_MERGE);
				labelEntropy.addItem(*node.label);
			}
		}
	}

	/// Get DAG structure entropy object
	EntropyCalculator<int> &getDagStructureEntropy() {
		return dagStructureEntropy;
	}

	/// Get DAG pointer entropy object
	EntropyCalculator<int> &getDagPointerEntropy() {
		return dagPointerEntropy;
	}

	/// Get DAG node label entropy object
	EntropyCalculator<DataType> &getLabelEntropy() {
		return labelEntropy;
	}

	/// Get merge type entropy object
	EntropyCalculator<char> &getMergeEntropy() {
		return mergeEntropy;
	}


private:
	EntropyCalculator<int> dagStructureEntropy;
	EntropyCalculator<int> dagPointerEntropy;
	EntropyCalculator<DataType> labelEntropy;
	EntropyCalculator<char> mergeEntropy;
	const BinaryDag<DataType> &dag;
};

struct StringLabelEntropy {
	StringLabelEntropy(const Labels<std::string> &labels) : labels(labels), entropy() {}

	void calculate() {
		for (const std::string* label : labels.valueIndex) {
			if (label != NULL) {
				entropy.addSequence(label->cbegin(), label->cend());
			}
			entropy.addItem(0);
		}
	}

	/// Additional amount of information that needs to be stored, in bits
	/// (e.g. for mapping code points to symbols)
	int getExtraSize() {
		return entropy.numSymbols() * sizeof(std::string::value_type) * 8;
	}

	EntropyCalculator<std::string::value_type> &getEntropy() {
		return entropy;
	}

private:
	const Labels<std::string> &labels;
	EntropyCalculator<std::string::value_type> entropy;
};
