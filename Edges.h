#pragma once

#include <ostream>

// TODO maybe be really hacky and use first bit of headNode as valid flag?
// -> invalid edges have negative ID
struct TreeEdge {
	int headNode;
	bool valid;

	TreeEdge(): headNode(-1), valid(false) {}

	friend std::ostream& operator<<(std::ostream& os, const TreeEdge &edge) {
		return os << "(" << edge.headNode << ";"  << (edge.valid ? "t" : "f") << ")";
	}
};
