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
	using TStackT = std::stack<DStackT>;

	Navigator(const DAGType &dag): dag(dag), dagStack(), treeStack() {
		std::cout << dag << std::endl;

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
			dagStack = treeStack.top();
			treeStack.pop();
			return true;
		}
	}

	bool isLeaf() {
		/*if (dagStack.size() == 1) {
			// we haven't really started yet, special case
			return false;
		}*/
		DStackT stack(dagStack);
		bool prevLeft(stack.top().left);
		stack.pop();
		while (!stack.empty()) {
			NavigationRecord record = stack.top();
			MergeType mergeType = dag.nodes[record.nodeId].mergeType;

			if ((!prevLeft && (mergeType == VERT_NO_BBN || mergeType == HORZ_LEFT_BBN)) || // b or c from the right
				(prevLeft && mergeType == HORZ_RIGHT_BBN) || // d from the left
				mergeType == HORZ_NO_BBN ||  // type e, either side
				(record.nodeId == (int)dag.nodes.size() - 1 && !prevLeft)) { // reached root from the right
				//std::cout << "iL: true (n=" << record.nodeId << " p=" << record.parentId << " l="  << prevLeft << " mT=" << mergeType << ") "<< std::flush;
				return true;
			}

			if (prevLeft && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				//std::cout << "iL: false (n=" << record.nodeId << " p=" << record.parentId << " l="  << prevLeft << " mT=" << mergeType << ") "<< std::flush;
				return false;
			}
			//std::cout << "iL: up up and away (n=" << record.nodeId << " p=" << record.parentId << " l="  << prevLeft << " mT=" << mergeType << ") "<< std::endl;
			prevLeft = record.left;
			stack.pop();
		}
		std::cout << "waah" << std::endl;
		assert(false);
	}

	bool firstChild() {
		//dumpDagStack();
		if (isLeaf()) {
			//std::cout << "fC: isLeaf. " << std::flush;
			return false;
		}
		treeStack.push(dagStack);
		bool prevLeft(dagStack.top().left);
		dagStack.pop();
		while (!dagStack.empty()) {
			NavigationRecord record = dagStack.top();
			MergeType mergeType = dag.nodes[record.nodeId].mergeType;

			if (prevLeft && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				break;
			}
			prevLeft = record.left;
			dagStack.pop();
		}
		int nodeId(dagStack.top().nodeId);
		int nextNode = dag.nodes[nodeId].right;
		dagStack.push(NavigationRecord(nextNode, nodeId, false));
		//std::cout << "fC: pushing " << dagStack.top() << std::endl;

		while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
			dagStack.push(NavigationRecord(nextNode, nodeId, true));
			//std::cout << "fC: pushing " << dagStack.top() << std::endl;
		}
		return true;
	}

	bool nextSibling() {
		//dumpDagStack();

		bool prevLeft(dagStack.top().left);
		bool alreadyHadCDE(false);
		dagStack.pop();
		while (!dagStack.empty()) {
			NavigationRecord record = dagStack.top();
			MergeType mergeType = dag.nodes[record.nodeId].mergeType;
			if (prevLeft && (mergeType == HORZ_LEFT_BBN || mergeType == HORZ_RIGHT_BBN || mergeType == HORZ_NO_BBN)) {
				break;
			}
			if ((!prevLeft || alreadyHadCDE) && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
				// a or b from right, or from either side after CDE => abort
				return false;
			}

			alreadyHadCDE |= (mergeType == HORZ_LEFT_BBN || mergeType == HORZ_RIGHT_BBN || mergeType == HORZ_NO_BBN);
			prevLeft = record.left;
			dagStack.pop();
		}

		if (dagStack.empty()) {
			return false;
		}

		int nodeId = dagStack.top().nodeId;
		int nextNode = dag.nodes[nodeId].right;
		dagStack.push(NavigationRecord(nextNode, nodeId, false));
		//std::cout << "nS: pushing " << dagStack.top() << std::endl;

		while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
			dagStack.push(NavigationRecord(nextNode, nodeId, true));
			//std::cout << "nS: pushing " << dagStack.top() << std::endl;
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

private:
	const DAGType &dag;
	DStackT dagStack;
	TStackT treeStack;
};