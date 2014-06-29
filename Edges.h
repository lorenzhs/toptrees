#pragma once

#include <ostream>

struct TreeEdge {
	int headNode;
	bool valid;

	TreeEdge(): headNode(-1), valid(false) {}

	friend std::ostream& operator<<(std::ostream& os, const TreeEdge &edge) {
		return os << "(" << edge.headNode << ";"  << (edge.valid ? "t" : "f") << ")";
	}
};
