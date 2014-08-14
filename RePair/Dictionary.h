#pragma once

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

	DataType nextIndex;
	std::unordered_map<DataType, std::pair<DataType, DataType>> dict;
};

}