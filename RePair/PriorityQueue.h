#pragma once

#include <cassert>
#include <vector>

#include "PQEntry.h"

namespace RePair {

struct PriorityQueue {
	PriorityQueue(const int size = 0) : maxIndex(-1), entries(size) {}

	void init(const int size) {
		entries.resize(size);
	}

	void clear() {
		entries.clear();
	}

	bool addEntry(PQEntry *entry) {
		assert(entry != nullptr);
		int index(getIndex(entry));

		if (index >= 0) {
			maxIndex = std::max(index, maxIndex);
			entry->clearFlag();
			entries[index] = entry->insertInto(entries[index]);
		}

		return index >= 0;
	}

	void removeEntry(PQEntry *entry) {
		assert(entry != nullptr);
		int index(getIndex(entry));
		assert(index >= 0);

		entry->setFlag();
		entries[index] = entry->removeFrom(entries[index]);
	}

	PQEntry* popMaxEntry() {
		PQEntry *max = nullptr;

		if (!empty()) {
			max = entries[maxIndex];
			entries[maxIndex] = max->removeFrom(entries[maxIndex]);
		}
		return max;
	}

	bool empty() {
		while (maxIndex >= 0 && entries[maxIndex] == nullptr) {
			--maxIndex;
		}
		return maxIndex < 0;
	}

private:
	int getIndex(PQEntry *entry) const {
		assert(entry != nullptr);
		return std::min(entry->getCount() - 2, (int)entries.size() - 1);
	}

	int maxIndex;
	std::vector<PQEntry*> entries;
};

}