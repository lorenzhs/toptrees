#pragma once

#include <cmath>
#include <vector>

#include "Dictionary.h"
#include "HashTable.h"
#include "PQEntry.h"
#include "PriorityQueue.h"
#include "Records.h"

namespace RePair {

template <typename DataType, typename InputType>
struct RePair {
	RePair(std::vector<InputType> &data) : records(data), hashTable(records), queue(), workingEntries(nullptr), dictionary(records) {}

	void compress() {
		int maxCount(fillHashTable());
		int queueSize(std::min(maxCount - 1, (int)sqrt(records.symbolCount)));
		queue.init(queueSize);

		fillQueue();

		while (!queue.empty()) {
			PQEntry *max = queue.popMaxEntry();
			hashTable.remove(max);
			const int index(max->index);
			const DataType first(records.text[index]), second(records.nextSymbol(index));
			const DataType newSymbol = dictionary.addPair(first, second);

			passOne(index, newSymbol, first, second);
			passTwo(index, newSymbol);
		}

		// not needed any more
		queue.clear();
		hashTable.clear();

		// return text.collapse();
	}

protected:
	int fillHashTable() {
		int maxCount(1);
		for (int index(1), nextIndex; index < records.symbolCount; index = nextIndex) {
			nextIndex = records.nextIndex(index);
			int prevOccurrence(records.prev[index]);
			if (prevOccurrence > nextIndex) {
				int count(1);
				for (int i = records.nextNonOverlappingOccurrence(index); i != index; i = records.nextNonOverlappingOccurrence(i)) {
					++count;
				}
				assert(count > 1);
				maxCount = std::max(maxCount, count);
				pqentries.emplace_back(PQEntry(index, count));

				PQEntry *entry(&pqentries.back());
				hashTable.insert(entry);
				workingEntries = entry->insertBefore(workingEntries);
			}
		}

		return maxCount;
	}

	void fillQueue() {
		while (workingEntries != nullptr) {
			PQEntry *entry = workingEntries;
			workingEntries = entry->removeFrom(workingEntries);
			bool addedToQueue = queue.addEntry(entry);
			assert(addedToQueue);
			(void) addedToQueue; // make compiler happy
		}
	}

	// replace pair ab with A
	void passOne(const int firstReplacement, const DataType A, const DataType a, const DataType b) {
		int endIndexOfFirstAA(-1), nextReplacement(firstReplacement);
		bool babyFlag(false), xabaFlag(false); // special case flags

		do {
			int xIndex(records.prevIndex(nextReplacement)),
				aIndex(nextReplacement),
				bIndex(records.nextIndex(aIndex)),
				yIndex(records.nextIndex(bIndex));
			const DataType x(records.text[xIndex]), y(records.text[yIndex]);
			nextReplacement = records.nextNonOverlappingOccurrence(aIndex);

			bool create_xA_Entry;
			if (x == A) { // check for xabab case
				if (endIndexOfFirstAA < 0)
					endIndexOfFirstAA = aIndex;
				create_xA_Entry = endIndexOfFirstAA < xIndex;
			} else {
				create_xA_Entry = removeIndex(xIndex);
				if (x == b) {
					create_xA_Entry = babyFlag;
					babyFlag = true;
				}
			}

			bool create_Ay_Entry = removeIndex(bIndex);
			if (y == a) {
				if (nextReplacement == yIndex) {
					create_Ay_Entry = false;
				} else {
					create_Ay_Entry = xabaFlag;
					xabaFlag = true;
				}
			}

			records.replacePair(aIndex, A);

			if (create_xA_Entry) {
				createEntryIfNotExists(xIndex);
			}

			if (create_Ay_Entry) {
				createEntryIfNotExists(aIndex);
			}
		} while (nextReplacement != firstReplacement);
	}

	bool removeIndex(const int index) {
		bool seen(false);
		PQEntry *entry(hashTable.find(index));
		if (entry == nullptr) {
			records.remove(index);
		} else {
			seen = entry->getFlag();
			if (!seen) {
				queue.removeEntry(entry);
				workingEntries = entry->insertBefore(workingEntries);
			}

			if (entry->index == index) {
				entry->index = records.next[index];
			}

			int countDelta = records.remove(index);
			entry->changeCount(countDelta);

			// no more occurences? kill it!
			if (entry->getCount() < 1) {
				workingEntries = entry->removeFrom(workingEntries);
				hashTable.remove(entry);
			}
		}
		return seen;
	}

	void createEntryIfNotExists(const int index) {
		if (hashTable.find(index) == nullptr) {
			pqentries.emplace_back(PQEntry(index, 0));
			PQEntry *entry(&pqentries.back());
			hashTable.insert(entry);
			workingEntries = entry->insertBefore(workingEntries);
		}
	}

	void passTwo(int nextIndex, const DataType A) {
		int lastNonOverlappingAA(-1), i;

		do {
			i = nextIndex;
			nextIndex = records.next[i];
			records.remove(i);

			int xIndex(records.prevIndex(i)), countDeltaForAy(1);
			DataType x(records.text[xIndex]), y(records.nextSymbol(i));

			if (y == A) {
				if (lastNonOverlappingAA != xIndex)
					lastNonOverlappingAA = i;
				else
					countDeltaForAy = 0;
			}
			if (x != A)
				addIndex(xIndex, 1);
			addIndex(i, countDeltaForAy);
		} while (i != nextIndex);

		moveWorkingEntriesBackToQueue();
	}

	void addIndex(const int index, const int countIncrement) {
		PQEntry *entry = hashTable.find(index);
		if (entry != nullptr) {
			if (entry->getCount() == 0)
				entry->index = index;
			else
				records.insertBefore(index, entry->index);
			entry->changeCount(countIncrement);
		}
	}

	void moveWorkingEntriesBackToQueue() {
		while (workingEntries != nullptr) {
			PQEntry *entry = workingEntries;
			workingEntries = entry->removeFrom(workingEntries);

			if (!queue.addEntry(entry))
				hashTable.remove(entry);
		}
	}

protected:
	std::vector<PQEntry> pqentries;
	Records<DataType> records;
	HashTable<DataType> hashTable;
	PriorityQueue queue;
	PQEntry *workingEntries;
	Dictionary<DataType> dictionary;
};

}