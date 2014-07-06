#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "Nodes.h"

using std::vector;
using std::function;

template<typename DataType>
class BinaryDag {
public:
	BinaryDag(const int n = 0): nodes() {
		nodes.reserve(n);
		// add a dummy element that is guaranteed to not appear
		// (we assume that -1 is used for leaves, never -2, except for this dummy)
		nodes.emplace_back(-2, -2, (std::string*)NULL);
	}

	int addNode(int left, int right, DataType *label) {
		nodes.emplace_back(left, right, label);
		return (nodes.size() - 1);
	}

	void popNode() {
		nodes.pop_back();
	}

	int addNodes(const int n) {
		nodes.resize(nodes.size() + n);
		return nodes.size() - n;
	}

	int countEdges() const {
		int count(0);
		for (const DagNode<DataType> &node : nodes) {
			count += (node.left >= 0);
			count += (node.right >= 0);
		}
		return count;
	}

	template<typename T>
	void inPostOrder(const function<T (const int, const T, const T)> &callback) {
		traverseDagPostOrder(nodes.size() - 1, callback);
	}

	template<typename T>
	T traverseDagPostOrder(const int nodeId, const function<T (const int, const T, const T)> &callback) {
		assert (nodeId != 0);  // 0 is the dummy not and should not be reachable
		DagNode<DataType> &node = nodes[nodeId];
		T left(-1), right(-1);
		if (node.left >= 0) {
			left = traverseDagPostOrder(node.left, callback);
		}
		if (node.right >= 0) {
			right = traverseDagPostOrder(node.right, callback);
		}
		return callback(nodeId, left, right);
	}

	friend std::ostream& operator<<(std::ostream& os, const BinaryDag<DataType> &dag) {
		os << "Binary Dag with " << dag.nodes.size() << " nodes";
		for (uint i = 0; i < dag.nodes.size(); ++i) {
			os << "; " << i << "=" << dag.nodes[i];
		}
		return os;
	}

	vector<DagNode<DataType>> nodes;
};
