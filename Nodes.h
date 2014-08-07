#pragma once

#include <ostream>

#include "Common.h"

/// A node for an OrderedTree
/**
 * Node type for use in OrderedTree
 * You can write other nodes that suit your needs
 *
 * Note that the member functions do not distinguish between valid
 * and invalid edges!
 */
struct TreeNode {
	int firstEdgeIndex;
	int lastEdgeIndex;
	int parent;
	int lastMergedIn;
	uint hash;

	TreeNode() : firstEdgeIndex(-1), lastEdgeIndex(-1), parent(-1), lastMergedIn(-1), hash(0) {}

	/// Get the number of outgoing edges (both valid and invalid)
	int numEdges() const {
		return lastEdgeIndex - firstEdgeIndex + 1;
	}

	/// Check whether the node is a leaf, i.e., has no outgoing edges
	bool isLeaf() const {
		return lastEdgeIndex < firstEdgeIndex;
	}

	// These are faster than comparing numEdges against a constant

	/// Check wether the node has only one child. Does not check edge validity
	/// \return true iff the node has exactly one outgoing edge
	bool hasOnlyOneChild() const {
		return firstEdgeIndex == lastEdgeIndex;
	}

	/// Check wether the node has outgoing edges (valid or invalid)
	int hasChildren() const {
		return firstEdgeIndex <= lastEdgeIndex;
	}

	/// Check wether the node has at last two outgoing edges (valid or invalid)
	bool hasMoreThanOneChild() const {
		return firstEdgeIndex < lastEdgeIndex;
	}

	friend std::ostream &operator<<(std::ostream &os, const TreeNode &node) {
		return os << "(" << node.parent << ";" << node.firstEdgeIndex << "→" << node.lastEdgeIndex
			//<< ";"  << node.lastMergedIn
			 << ")";
	}
};

/// This is a node type for use in a DAG
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
