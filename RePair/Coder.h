#pragma once

#include <vector>

#include "../Common.h"
#include "../Huffman.h"
#include "Dictionary.h"

namespace RePair {

template <typename DataType>
struct Coder {
	Coder(std::vector<DataType> &output, Dictionary<DataType> &dict) : output(output), dict(dict), huff() {}

	void compute() {
		huff.addItem(dict.getFirstIndex());  // encode gap for primitive symbols
		huff.addItem(dict.numSymbols());  // encode length of table
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

	long long getBitsNeeded() {
		// don't need to code the
		return huff.getBitsNeeded() + huff.getBitsForTableLabels();
	}

	std::vector<DataType> &output;
	Dictionary<DataType> &dict;
	HuffmanBuilder<DataType> huff;
};

}