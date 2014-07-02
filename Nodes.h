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

template<typename T>
struct DagNode {
	int left;
	int right;
	int inDegree;
	T* label;

	DagNode(): left(-1), right(-1), inDegree(0), label(NULL) {}
	DagNode(int l, int r, T *la): left(l), right(r), inDegree(0), label(la) {}
	DagNode(const DagNode<T> &other): left(other.left), right(other.right), inDegree(other.inDegree), label(other.label) {}

	friend std::ostream& operator<<(std::ostream& os, const DagNode &node) {
		return os << "(" << node.left << ";"  << node.right << ";" << node.inDegree << ";" << (node.label == NULL ? "NULL" : *node.label) << ")";
	}
};
