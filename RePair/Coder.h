#pragma once

#include <vector>

#include "../Common.h"
#include "../Huffman.h"
#include "Dictionary.h"

namespace RePair {

/// encode RePair output
template <typename DataType>
struct Coder {
	Coder(std::vector<DataType> &output, Dictionary<DataType> &dict) : bitsForInputMapping(0), output(output), dict(dict), huff() {}

	void compute() {
		huff.addItem(dict.getFirstIndex());  // encode gap for primitive symbols
		huff.addItem(dict.size());  // encode length of table
		// encode pairs from dictionary
		for (DataType i = dict.getFirstIndex(); i < dict.numSymbols(); ++i) {
			auto pair = dict.getProduction(i);
			huff.addItem(pair.first);
			huff.addItem(pair.second);
		}

		// encode RePair output
		for (DataType elem : output) {
			huff.addItem(elem);
		}

		huff.construct();
	}

	template <typename InputType>
	void codeInputMapping(std::unordered_map<InputType, InputType> &mapping) {
		InputType maxSymbol(0);
		for (auto it = mapping.begin(); it != mapping.end(); ++it) {
			maxSymbol = std::max(maxSymbol, it->second);
		}
		const int bitsPerSymbol(log2(maxSymbol));
		// code table as fixed-width numbers plus its size
		bitsForInputMapping = mapping.size() * bitsPerSymbol + sizeof(InputType)*8;
	}

	long long getBitsNeeded() {
		// don't need to code the
		return huff.getBitsNeeded() + huff.getBitsForTableLabels() + bitsForInputMapping;
	}

	long long bitsForInputMapping;
	std::vector<DataType> &output;
	Dictionary<DataType> &dict;
	HuffmanBuilder<DataType> huff;
};

}