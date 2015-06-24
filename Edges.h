#pragma once

#include <ostream>

/// Edge type for use with an OrderedTree
struct TreeEdge {
	// Pack them both into one unsigned int
	// headNode is theoretically an int, but since the sign bit
	// is unused anyway (all nodes have ID >= 0), using it for
	// the valid flag is safe.
	unsigned int valid : 1;
	unsigned int headNode : 31;

	TreeEdge() : valid(false), headNode(0) {}

	friend std::ostream &operator<<(std::ostream &os, const TreeEdge &edge) {
		return os << "(" << edge.headNode << ";" << (edge.valid ? "t" : "f") << ")";
	}
};
