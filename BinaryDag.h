#pragma once

#include <cassert>
#include <vector>

#include "Nodes.h"

using std::vector;

template<typename DataType>
class BinaryDag {
public:
	BinaryDag(const int n = 0): nodes() {
		nodes.reserve(n);
		std::string* s = new std::string("DUMMY");
		nodes.emplace_back(-1, -1, s);
	}

	int nodeId(DagNode<DataType> *node) const {
		return node - nodes.data();
	}

	int addNode(int left, int right, DataType *label) {
		nodes.emplace_back(left, right, label);
		if (left != -1) {
			nodes[left].inDegree++;
		}
		if (right != -1) {
			nodes[right].inDegree++;
		}
		return (nodes.size() - 1);
	}

	void popNode() {
		nodes.pop_back();
	}

	int addNodes(const int n) {
		nodes.resize(nodes.size() + n);
		return nodes.size() - n;
	}

	void setLeftEdge(const int from, const int to) {
		assert (0 <= from && from < (int) nodes.size() && 0 <= to && to < (int) nodes.size());
		nodes[from].left = to;
		nodes[to].inDegree++;
	}

	void setRightEdge(const int from, const int to) {
		assert (0 <= from && from < (int) nodes.size() && 0 <= to && to < (int) nodes.size());
		nodes[from].right = to;
		nodes[to].inDegree++;
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
