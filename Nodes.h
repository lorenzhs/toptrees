#pragma once

#include <ostream>

#include "Common.h"

struct TreeNode {
	int firstEdgeIndex;
	int lastEdgeIndex;
	int parent;
	int lastMergedIn;

	TreeNode() : firstEdgeIndex(-1), lastEdgeIndex(-1), parent(-1), lastMergedIn(-1) {}

	int numEdges() const {
		return lastEdgeIndex - firstEdgeIndex + 1;
	}

	bool isLeaf() const {
		return lastEdgeIndex < firstEdgeIndex;
	}

	// These are faster than comparing numEdges against a constant
	bool hasOnlyOneChild() const {
		return firstEdgeIndex == lastEdgeIndex;
	}

	int hasChildren() const {
		return firstEdgeIndex <= lastEdgeIndex;
	}

	bool hasMoreThanOneChild() const {
		return firstEdgeIndex < lastEdgeIndex;
	}

	friend std::ostream &operator<<(std::ostream &os, const TreeNode &node) {
		return os << "(" << node.parent << ";" << node.firstEdgeIndex << "→" << node.lastEdgeIndex
			//<< ";"  << node.lastMergedIn
			 << ")";
	}
};

template <typename T>
struct DagNode {
	int left;
	int right;
	int inDegree;
	MergeType mergeType;
	const T *label;

	DagNode() : left(-1), right(-1), inDegree(0), mergeType(NO_MERGE), label(NULL) {}
	DagNode(int l, int r, const T *la, MergeType t) : left(l), right(r), inDegree(0), mergeType(t), label(la) {}
	DagNode(const DagNode<T> &other)
		: left(other.left),
		  right(other.right),
		  inDegree(other.inDegree),
		  mergeType(other.mergeType),
		  label(other.label) {}

	friend std::ostream &operator<<(std::ostream &os, const DagNode &node) {
		os << "(" << node.left << ";" << node.right << ";" << node.inDegree << ";";
		if (node.label == NULL)
			os << "NULL";
		else
			os << *node.label;
		return os << ")";
	}
};
