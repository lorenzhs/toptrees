#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

namespace RePair {

// I plead guilty for choosing a really bad pun as this helper's name
template <typename InType>
struct Prepair {
	static void prepare(std::vector<InType> &vec, std::unordered_map<InType, InType> &transformations) {
		transformations.clear();
		InType nextSymbol(0);

		for (auto it = vec.begin(); it != vec.end(); ++it) {
			auto replacement = transformations.find(*it);
			if (replacement == transformations.end()) {
				// new symbol
				transformations[*it] = nextSymbol;
				*it = nextSymbol;
				nextSymbol += 1;
			} else {
				assert(*it == replacement->first);
				*it = replacement->second;
			}
		}
	}
};

}