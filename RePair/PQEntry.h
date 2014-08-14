#pragma once

#include <cassert>
#include <ostream>

namespace RePair {

struct PQEntry {
	PQEntry() : index(0), count(FLAG_MASK), nextEntry(nullptr), prevEntry(nullptr) {}
	PQEntry(const int index, const int cnt) : index(index), count(cnt | FLAG_MASK), nextEntry(nullptr), prevEntry(nullptr) {}

	PQEntry* insertInto(PQEntry *list) {
		assert(prevEntry == nullptr && nextEntry == nullptr && (list == nullptr || list->prevEntry == nullptr));
		PQEntry *prev(nullptr), *next(list);

		while (next != nullptr && getCount() < next->getCount()) {
			prev = next;
			next = prev->nextEntry;
			assert(next->prevEntry == prev);
		}

		if (next != nullptr) {
			nextEntry = next;
			next->prevEntry = this;
		}
		if (prev != nullptr) {
			prevEntry = prev;
			prev->nextEntry = this;
			return list;
		} else {
			return this;
		}
	}

	PQEntry* insertBefore(PQEntry *next) {
		assert(prevEntry == nullptr && nextEntry == nullptr);
		if (next != nullptr) {
			assert(next->prevEntry == nullptr);
			nextEntry = next;
			next->prevEntry = this;
		}
		return this;
	}

	PQEntry* removeFrom(PQEntry *first) {
		assert(first != nullptr);
		if (nextEntry != nullptr) {
			assert(nextEntry->prevEntry == this);
			nextEntry->prevEntry = prevEntry;
		}
		if (prevEntry != nullptr) {
			assert(prevEntry->nextEntry == this);
			prevEntry->nextEntry = nextEntry;
		} else { // this was the first item
			assert(first == this);
			first = nextEntry;
		}

		nextEntry = nullptr;
		prevEntry = nullptr;

		return first;
	}

	int getCount() const {
		return count & ~FLAG_MASK;
	}

	bool getFlag() const {
		return (count & FLAG_MASK);
	}

	void clearFlag() {
		count &= ~FLAG_MASK;
	}

	void setFlag() {
		count |= FLAG_MASK;
	}

	void changeCount(const int delta) {
		count += delta;
	}

	friend std::ostream &operator<<(std::ostream &os, const PQEntry &entry) {
		return os << "(c=" << entry.getCount() << " f=" << entry.getFlag() << " i=" << entry.index << " next=" << entry.nextEntry << " prev=" << entry.prevEntry << ")";
	}

public:
	int index;
protected:
	// flag states whether the entry is in the PQ and stored in MSB of count
	static const int FLAG_MASK = 0x40000000;
	int count;
	PQEntry *nextEntry, *prevEntry;
};

}