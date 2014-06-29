#pragma once

#include <ostream>

struct TreeNode {
	int firstEdgeIndex;
	int lastEdgeIndex;
	int parent;
	int lastMergedIn;

	TreeNode(): firstEdgeIndex(-1), lastEdgeIndex(-1), parent(-1), lastMergedIn(-1) {}

	int numEdges() const  {
		return lastEdgeIndex - firstEdgeIndex + 1;
	}

	int isLeaf() const {
		return (lastEdgeIndex + 1) == firstEdgeIndex;
	}

	friend std::ostream& operator<<(std::ostream& os, const TreeNode &node) {
		return os << "(" << node.parent << ";"  << node.firstEdgeIndex << "→" << node.lastEdgeIndex << ";" << node.lastMergedIn << ")";
	}
};
