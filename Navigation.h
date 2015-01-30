#pragma once

#include <cassert>
#include <stack>

#include "BinaryDag.h"

struct NavigationRecord {
	int nodeId;
	int parentId;
	bool left;
	NavigationRecord() : nodeId(-1), parentId(-1), left(true) {}
	NavigationRecord(int node, int parent, bool left) : nodeId(node), parentId(parent), left(left) {}

	friend std::ostream &operator<<(std::ostream &os, const NavigationRecord &record) {
		return os << "(" << record.nodeId << ";" << record.parentId << ";" << record.left << ")";
	}
};

template <typename DataType>
class Navigator {
public:
	using DAGType = BinaryDag<DataType>;
	using DStackT = std::stack<NavigationRecord>;
	using TStackT = std::deque<DStackT>;

	Navigator(const DAGType &dag): dag(dag), dagStack(), treeStack(), maxTreeStackSize(0) {
		if (verbose) std::cout << dag << std::endl;

		int nodeId = -1;
		int nextNode = (int)dag.nodes.size() - 1;
		while (nextNode > 0) {
			dagStack.push(NavigationRecord(nextNode, nodeId, true));
			nodeId = nextNode;
			nextNode = dag.nodes[nodeId].left;
		}
	}

	const DataType* getLabel() const {
		return dag.nodes[dagStack.top().nodeId].label;
	}

	bool parent() {
		if (treeStack.empty()) {
			return false;
		} else {
			dagStack = treeStack.back();
			treeStack.pop_back();
			return true;
		}
	}

	bool isLeaf() {
		DStackT stack(dagStack);
		while (!stack.empty()) {
			NavigationRecord record = stack.top();
			MergeType mergeType = dag.nodes[record.parentId].mergeType;

			if ((!record.left && (mergeType == VERT_NO_BBN || mergeType == HORZ_LEFT_BBN)) || // b or c from the right
				(record.left && mergeType == HORZ_RIGHT_BBN) || // d from the left
				mergeType == HORZ_NO_BBN ||  // type e, either side
				(record.nodeId == (int)dag.nodes.size() - 1 && !record.left)) { // reached root from the right
				return true;
			}

			if (record.left && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				return false;
			}
			stack.pop();
		}
		assert(false);
		return false;
	}

	bool firstChild() {
		if (verbose) dumpDagStack();
		if (isLeaf()) {
			return false;
		}
		treeStack.push_back(dagStack);
		maxTreeStackSize = std::max(maxTreeStackSize, getTreeStackSize());
		while (!dagStack.empty()) {
			NavigationRecord record = dagStack.top();
			MergeType mergeType = dag.nodes[record.parentId].mergeType;

			if (record.left && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				break;
			}
			dagStack.pop();
		}
		int nodeId(dagStack.top().parentId);
		int nextNode = dag.nodes[nodeId].right;
		dagStack.pop();
		dagStack.push(NavigationRecord(nextNode, nodeId, false));
		if (verbose) std::cout << "fC: pushing " << dagStack.top() << std::endl;

		while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
			dagStack.push(NavigationRecord(nextNode, nodeId, true));
			if (verbose) std::cout << "fC: pushing " << dagStack.top() << std::endl;
		}
		return true;
	}

	bool nextSibling() {
		if (verbose) dumpDagStack();

		DStackT stack(dagStack);
		while (!stack.empty()) {
			NavigationRecord &record = stack.top();
			MergeType mergeType = dag.nodes[record.parentId].mergeType;
			if (record.left && (mergeType == HORZ_LEFT_BBN || mergeType == HORZ_RIGHT_BBN || mergeType == HORZ_NO_BBN)) {
				break;
			}
			if ((!record.left) && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				// a or b from right => abort
				return false;
			}

			stack.pop();
		}

		// No more next siblings in the tree, we've exhausted the stack
		if (stack.empty()) {
			return false;
		}

		dagStack = stack;

		int nodeId = dagStack.top().parentId;
		int nextNode = dag.nodes[nodeId].right;
		dagStack.pop();
		dagStack.push(NavigationRecord(nextNode, nodeId, false));
		if (verbose) std::cout << "nS: pushing " << dagStack.top() << std::endl;

		while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
			dagStack.push(NavigationRecord(nextNode, nodeId, true));
			if (verbose) std::cout << "nS: pushing " << dagStack.top() << std::endl;
		}
		return true;
	}

	void dumpDagStack() {
		DStackT stack(dagStack);
		std::cout << "DagStack: ";
		while (!stack.empty()) {
			std::cout << stack.top() << " :: ";
			stack.pop();
		}
		std::cout << std::endl;
	}

	long long getTreeStackSize() const {
		long long treeStackSize(0);
		for (const DStackT &stack : treeStack) {
			treeStackSize += stack.size();
		}
		treeStackSize *= sizeof(DStackT::value_type);
		return treeStackSize;
	}

	long long getMaxTreeStackSize() const {
		return maxTreeStackSize;
	}

private:
	const DAGType &dag;
	DStackT dagStack;
	TStackT treeStack;
	long long maxTreeStackSize;
	static const bool verbose = false;
};
