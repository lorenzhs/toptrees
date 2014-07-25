#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "Nodes.h"

using std::vector;
using std::function;

/// A binary DAG that is specialised to be a top tree's minimal DAG
template <typename DataType>
class BinaryDag {
public:
	/// Create a new binary DAG
	/// \param n number of node slots to allocate
	BinaryDag(const int n = 0) : nodes() {
		nodes.reserve(n);
		// add a dummy element that is guaranteed to not appear
		// (we assume that -1 is used for leaves, never -2, except for this dummy)
		nodes.emplace_back(-2, -2, (DataType *)NULL, NO_MERGE);
	}

	/// Add a node
	/// \param left left child
	/// \param right right child
	/// \param label a label pointer
	/// \param mergeType the original node's merge type (for use with a TopTree)
	int addNode(int left, int right, const DataType *label, MergeType mergeType) {
		nodes.emplace_back(left, right, label, mergeType);
		return (nodes.size() - 1);
	}

	/// Remove the last node
	void popNode() {
		nodes.pop_back();
	}

	/// Add multiple nodes
	/// \param n the number of nodes to add
	/// \return the ID of the first node added
	int addNodes(const int n) {
		nodes.resize(nodes.size() + n);
		return nodes.size() - n;
	}

	/// Count the number of edges in the DAG
	int countEdges() const {
		int count(0);
		for (const DagNode<DataType> &node : nodes) {
			count += (node.left >= 0);
			count += (node.right >= 0);
		}
		return count;
	}

	/// Traverse the dag in post-order
	/// \param callback a callback to be called with the node ID and the results of the calls to its children
	template <typename T>
	T inPostOrder(const function<T(const int, const T, const T)> &callback) const {
		return traverseDagPostOrder(nodes.size() - 1, callback);
	}

	/// Helper for inPostOrder(), you shouldn't need to call this directly
	template <typename T>
	T traverseDagPostOrder(const int nodeId, const function<T(const int, const T, const T)> &callback) const {
		assert(nodeId != 0); // 0 is the dummy not and should not be reachable
		const DagNode<DataType> &node = nodes[nodeId];
		T left(-1), right(-1);
		if (node.left >= 0) {
			left = traverseDagPostOrder(node.left, callback);
		}
		if (node.right >= 0) {
			right = traverseDagPostOrder(node.right, callback);
		}
		return callback(nodeId, left, right);
	}

	friend std::ostream &operator<<(std::ostream &os, const BinaryDag<DataType> &dag) {
		os << "Binary Dag with " << dag.nodes.size() << " nodes";
		for (uint i = 0; i < dag.nodes.size(); ++i) {
			os << "; " << i << "=" << dag.nodes[i];
		}
		return os;
	}

	vector<DagNode<DataType>> nodes;
};
