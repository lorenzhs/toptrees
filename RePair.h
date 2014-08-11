#pragma once

#include <cassert>
#include <ostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Common.h"

namespace RePair {

struct HashCombiner {
	uint operator()(const uint leftHash, const uint rightHash) const {
		return hash(leftHash, rightHash);
	}

	static uint hash(const uint leftHash, const uint rightHash) {
		uint seed(0);
		boost_hash_combine(seed, leftHash);
		boost_hash_combine(seed, rightHash);
		return seed;
	}
};

template <typename Pair>
struct Record {
	Record(const uint hash = 0) : hash(hash), frequency(0), occurrences() {}
	Record(const Record<Pair> &other) : hash(other.hash), frequency(other.frequency), occurrences(other.occurrences) {}
	const uint hash;
	uint frequency;
	std::vector<Pair> occurrences;

	friend std::ostream &operator<<(std::ostream &os, const Record<Pair> &record) {
		return os << "(" << record.frequency << "x" << record.hash << ")";
	}
};

template <typename Pair>
struct Records {
	Records() : records(1) {}
	int add(uint hash) {
		// add a dummy record for std::unordered_map stuff
		records.emplace_back(Record<Pair>(hash));
		return records.size()-1;
	}

	Record<Pair>& operator[](typename std::vector<Record<Pair>>::size_type index) {
		return records[index];
	}

	const Pair& operator[](typename std::vector<Record<Pair>>::size_type index) const {
		return records[index];
	}

	std::vector<Record<Pair>> records;
};

template <typename Pair>
struct RecordFrequencyComparator {
	bool operator()(const Record<Pair> *record, const Record<Pair> *other) {
		return record->frequency > other->frequency;
	}
};

template <typename Pair>
struct PriorityQueue {
	PriorityQueue(const int size = 0) : lastNonEmptyList(-1), lists(size), frequentRecords() {}

	void init(const int size) {
		lists.resize(size);
	}

	void insert(Record<Pair>* record) {
		assert(record->frequency >= 2);
		const uint bucket = record->frequency - 2;
		if (bucket < lists.size()) {
			lastNonEmptyList = std::max(lastNonEmptyList, (int)bucket);
			lists[bucket].insert(record);
		} else {
			lastNonEmptyList = lists.size();
			frequentRecords.insert(record);
		}
	}

	bool empty() const {
		return lastNonEmptyList < 0;
	}

	Record<Pair> *popMostFrequentRecord() {
		Record<Pair> *result;
		assert(lastNonEmptyList >= 0);
		if (lastNonEmptyList == (int)lists.size()) {
			assert(!frequentRecords.empty());
			result = *frequentRecords.begin();
			frequentRecords.erase(frequentRecords.begin());
			if (frequentRecords.empty()) {
				lastNonEmptyList--;
			}
		} else {
			auto &list = lists[lastNonEmptyList];
			assert(!list.empty());
			result = *list.begin();
			list.erase(list.begin());
		}

		findNextNonEmptyList();

		return result;
	}

	void decrementFrequency(Record<Pair> *record) {
		assert(lastNonEmptyList >= 0);
		const uint bucket(record->frequency - 2);
		if (bucket < lists.size()) {
			lists[bucket].erase(record);
			record->frequency--;
			if (bucket > 0) {
				lists[bucket - 1].insert(record);
			}
		} else {
			frequentRecords.erase(record);
			record->frequency--;
			if (record->frequency < lists.size()) {
				if (record->frequency >= 2) {
					lists[record->frequency - 2].insert(record);
				}
				if (frequentRecords.empty()) {
					lastNonEmptyList--;
				}
			} else {
				frequentRecords.insert(record);
			}
		}

		findNextNonEmptyList();
	}

	friend std::ostream &operator<<(std::ostream &os, const PriorityQueue<Pair> &queue) {
		os << "PriorityQueue with " << queue.lists.size() << " + 1 buckets, lastNonEmptyList = " << queue.lastNonEmptyList << std::endl;
		for (auto &list : queue.lists) {
			os << "List " << (&list - queue.lists.data()) << ":";
			for (Record<Pair> *record : list) {
				os << " " << *record;
			}
			os << std::endl;
		}
		os << "Frequent records list:";
		for (Record<Pair> *record : queue.frequentRecords) {
			os << " " << *record;
		}
		return os << std::endl;
	}

private:
	void findNextNonEmptyList() {
		if (lastNonEmptyList == (int) lists.size()) {
			assert(!frequentRecords.empty());
			return;
		}
		while(lastNonEmptyList >= 0 && lists[lastNonEmptyList].empty()) {
			lastNonEmptyList--;
		}
		assert(lastNonEmptyList == -1 || !lists[lastNonEmptyList].empty());
	}

	int lastNonEmptyList;
	std::vector<std::unordered_set<Record<Pair>*>> lists;
	std::set<Record<Pair>*, RecordFrequencyComparator<Pair>> frequentRecords;
};

template <typename Pair>
struct HashMap {
	HashMap(Records<Pair> &records) : records(records) {}

	void add(uint hash, const Pair &pair) {
		uint &index = recordMap[hash];
		if (index == 0) {
			index = records.add(hash);
		}
		assert(index != 0);
		records[index].frequency++;
		records[index].occurrences.push_back(pair);
	}

	void populatePQ(PriorityQueue<Pair> &queue) {
		for (auto it = recordMap.begin(); it != recordMap.end(); ++it) {
			if (it->second != 0 && records[it->second].frequency >= 2) {
				queue.insert(&records[it->second]);
			}
		}
	}

	friend std::ostream &operator<<(std::ostream &os, const HashMap<Pair> &hashMap) {
		os << "HashMap with " << hashMap.recordMap.size() << " different hashes" << std::endl;
		for (auto elem : hashMap.recordMap) {
			os << "Hash " << elem.first << " record " << hashMap.records[elem.second] << std::endl;
		}
		os << "Records:";
		for (Record<Pair> &record : hashMap.records.records) {
			os << "  " << record;
		}
		os << std::endl;
		return os;
	}

	std::unordered_map<uint, uint> recordMap;
	Records<Pair> &records;
};

}