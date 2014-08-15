#pragma once

#include <ostream>
#include <unordered_map>

#include "Records.h"

namespace RePair {

template <typename DataType>
struct Dictionary {
	Dictionary(Records<DataType> &initialContent) : nextIndex(0) {
		for (const auto symbol : initialContent.text) {
			nextIndex = std::max(nextIndex, symbol);
		}
		nextIndex++;
	}

	DataType addPair(DataType first, DataType second) {
		dict[nextIndex++] = std::make_pair(first, second);
		return (nextIndex - 1);
	}

	friend std::ostream &operator<<(std::ostream &os, const Dictionary &dict) {
		os << "Dictionary with " << dict.dict.size() << " symbols:" << std::endl;
		for (auto &pair : dict.dict) {
			os << "\t" << pair.first << " -> (" << pair.second.first << "," << pair.second.second << ")" << std::endl;
		}
		return os;
	}

	DataType nextIndex;
	std::unordered_map<DataType, std::pair<DataType, DataType>> dict;
};

}