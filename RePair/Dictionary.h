#pragma once

#include <ostream>
#include <unordered_map>

#include "Records.h"

namespace RePair {

template <typename DataType>
struct Dictionary {
    Dictionary(Records<DataType> &initialContent) : nextIndex(0) {
        const int maxIndex = (int)initialContent.text.size() - 1;
        for (int i = 1; i < maxIndex; ++i) {
            firstIndex = std::max(firstIndex, initialContent.text[i]);
        }
        firstIndex++;
        nextIndex = firstIndex;
    }

    DataType addPair(DataType first, DataType second) {
        dict[nextIndex++] = std::make_pair(first, second);
        return (nextIndex - 1);
    }

    std::pair<DataType, DataType> getProduction(const DataType key) {
        return dict[key];
    }

    uint size() const {
        return dict.size();
    }

    DataType numSymbols() const {
        return nextIndex;
    }

    DataType getFirstIndex() const {
        return firstIndex;
    }

    friend std::ostream &operator<<(std::ostream &os, const Dictionary &dict) {
        os << "Dictionary with " << dict.dict.size() << " symbols:" << std::endl;
        for (auto &pair : dict.dict) {
            os << "\t" << pair.first << " -> (" << pair.second.first << "," << pair.second.second << ")" << std::endl;
        }
        return os;
    }

private:
    DataType nextIndex, firstIndex;
    std::unordered_map<DataType, std::pair<DataType, DataType>> dict;
};

}