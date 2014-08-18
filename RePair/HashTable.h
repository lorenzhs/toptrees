#pragma once

#include <vector>

#include "PQEntry.h"
#include "Records.h"

namespace RePair {

/// RePair hash table using hashing with open addressing and linear probing
template <typename DataType>
class HashTable {
public:
	HashTable(Records<DataType> &records) : freeSlots(records.symbolCount), lastHashIndex(0), table(freeSlots, nullptr), text(records) {}

	/// Find a PQEntry by its index
	PQEntry* find(const int index) {
		const int first(text.text[index]), second(text.nextSymbol(index));

		PQEntry *entry = table[lastHashIndex];
		if (entry == nullptr || !text.occursAt(entry->index, first, second)) {
			lastHashIndex = hashPair(first, second);
			do {
				lastHashIndex = (lastHashIndex + 1) % table.size();
				entry = table[lastHashIndex];
			} while (entry != nullptr && !text.occursAt(entry->index, first, second));
		}

		return entry;
	}

	/// Add a PQEntry into the hash table
	void insert(PQEntry *entry) {
		assert(freeSlots > 1);
		table[lookup(entry)] = entry;
		--freeSlots;
	}

	/// Delete a PQEntry from the hash table
	void remove(PQEntry *entry) {
		int index(lastHashIndex);
		if (table[index] != entry) {
			index = lookup(entry);
		}
		assert(table[index] == entry);
		table[index] = nullptr;
		++freeSlots;

		// rehash
		while (true) {
			index = (index + 1) % table.size();
			entry = table[index];
			if (entry == nullptr) {
				break;
			} else {
				table[index] = nullptr;
				table[lookup(entry)] = entry;
			}
		}
	}

	/// Clear everything from the hash table. It  won't be reusable afterwards,
	/// this is if you no longer need it and want to reclaim the memory
	void clear() {
		table.clear();
	}

	/// Hash two things. Maybe not the greatest hash function in the world.
	static DataType hashPair(const DataType a, const DataType b) {
		DataType res = a * (a + b + 1) + b * (b + 1);
		return (res < 0) ? -res : res;
	}

private:

	int lookup(PQEntry *entry) {
		lastHashIndex = hashEntry(entry);
		do {
			lastHashIndex = (lastHashIndex + 1) % table.size();
		} while(table[lastHashIndex] != nullptr && table[lastHashIndex] != entry);

		return lastHashIndex;
	}

	int hashEntry(PQEntry *entry) const {
		const DataType first(text.text[entry->index]), second(text.nextSymbol(entry->index));
		return hashPair(first, second);
	}

private:
	int freeSlots, lastHashIndex;
	std::vector<PQEntry*> table;
	Records<DataType> &text;
};

}