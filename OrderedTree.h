#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

#include "Common.h"
#include "Timer.h"

using std::cout;
using std::endl;
using std::flush;
using std::vector;

// #define awfulness
#define FORALL_NODES(tree, node) for (int node = 0; node < tree._numNodes; ++node)
#define FORALL_EDGES(tree, node, edge) for (int node = 0; node < tree._numNodes; ++node) for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)
#define FORALL_OUTGOING_EDGES(tree, node, edge) for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(u); ++edge)

template<typename NodeType, typename EdgeType>
class OrderedTree {
public:
	OrderedTree(const int n=0, const int m=0) {
		initialise(n, m);
	}

	EdgeType* firstEdge() {
		return edges.data();
	}
	EdgeType* firstEdge(const int u) {
		return edges.data() + nodes[u].firstEdgeIndex;
	}

	EdgeType* lastEdge() {
		return edges.data() + _numEdges - 1;
	}
	EdgeType* lastEdge(const int u) {
		return edges.data() + nodes[u].lastEdgeIndex;
	}

	// Get ID from pointer, woohoo for not doing this in Scala!
	int nodeId(const NodeType* node) {
		return (node - nodes.data());
	}
	int edgeId(const EdgeType* edge) {
		return (edge - edges.data());
	}

	// Add a node and return its id
	int addNode() {
		if (_firstFreeNode <= (int) nodes.size()) {
			nodes.resize(_firstFreeNode + 1);
		}
		nodes[_firstFreeNode].firstEdgeIndex = _firstFreeEdge;
		nodes[_firstFreeNode].lastEdgeIndex = _firstFreeEdge - 1;
		++_numNodes;
		return _firstFreeNode++;
	}

	// Add `n` nodes and return id of the first one
	int addNodes(const int n) {
		for (int i = 0; i < n; ++i) {
			addNode();
		}
		return _firstFreeNode - n;
	}

	void isolateNode(const int u) {
		for (EdgeType* edge = firstEdge(u); edge <= lastEdge(u); ++edge) {
			edge->valid = false;
		}
		nodes[u].lastEdgeIndex = nodes[u].firstEdgeIndex - 1;
	}

	EdgeType* addEdge(const int from, const int to) {
		NodeType& node = nodes[from];
		int newId = node.lastEdgeIndex + 1;
		// Check for space to the right
		if ((unsigned int)newId < edges.size() && !edges[newId].valid) {
			node.lastEdgeIndex++;
			if (newId == _firstFreeEdge) {
				_firstFreeEdge++;
			}
			return _prepareEdge(newId, from, to);
		}

		// check for space to the left
		newId = node.firstEdgeIndex - 1;
		if (newId > 0 && !edges[newId].valid) {
			node.firstEdgeIndex--;
			for (int i = node.firstEdgeIndex; i < node.lastEdgeIndex; ++i) {
				edges[i] = edges[i+1];
			}
			return _prepareEdge(node.lastEdgeIndex + 1, from, to);
		}

		if (node.numEdges() >= (int)edges.size() - _firstFreeEdge) {
			// allocate more space. this allocates too much if the node is at the end but we don't really care
			edges.resize(_firstFreeEdge + node.numEdges() + 1);
		}

		if (node.lastEdgeIndex == _firstFreeEdge - 1) {
			// We're already at the end
			newId = _firstFreeEdge;
		} else {
			// move node's edges to the end
			newId = _firstFreeEdge + node.numEdges();
			for (int i = 0; i < node.numEdges(); ++i) {
				edges[_firstFreeEdge + i] = edges[node.firstEdgeIndex + i];
				edges[node.firstEdgeIndex + i].valid = false;
			}
			node.firstEdgeIndex = _firstFreeEdge;
		}

		node.lastEdgeIndex = newId;
		_firstFreeEdge = newId + 1;
		edges[newId] = EdgeType();
		return _prepareEdge(newId, from, to);
	}

	void removeEdge(const int from, const int edge, const bool compact = true) {
		assert(edges[edge].valid);
		edges[edge].valid = false;
		NodeType& node = nodes[from];
		if (edge == node.lastEdgeIndex) {
			node.lastEdgeIndex--;
		} else if (edge == node.firstEdgeIndex) {
			node.firstEdgeIndex++;
		} else if (compact) {
			// because this is an ordered tree, we have to move edges around :(
			if ((edge - node.firstEdgeIndex) >= (node.lastEdgeIndex - edge)) {
				// there are morge edges on the left => move right side to the left
				for (int i = edge; i < node.lastEdgeIndex; ++i) {
					edges[i] = edges[i+1];
				}
				edges[node.lastEdgeIndex--].valid = false;
			} else {
				// move edges on the left side to the right
				for (int i = edge; i > node.firstEdgeIndex; --i) {
					edges[i] = edges[i-1];
				}
				edges[node.firstEdgeIndex++].valid = false;
			}
		}
		_numEdges--;
	}

	void removeEdgeTo(const int from, const int to, const bool compact = true) {
		NodeType& node = nodes[from];
		for (int i = node.firstEdgeIndex; i <= node.lastEdgeIndex; ++i) {
			if (edges[i].headNode == to) {
				assert(nodes[edges[i].headNode].parent == from);
				removeEdge(from, i, compact);
				return;
			}
		}
	}

	void doMerges(const std::function<void (const int, const int, const int, const MergeType)> &mergeCallback, const bool verbose = true) {
		int iteration = 0;
		Timer timer;
		while (_numEdges > 1) {
			if (verbose) cout << "Iteration " << iteration << ": horizontal merges... " << flush;
			horizontalMerges(iteration, mergeCallback);
			if (verbose) cout << timer.getAndReset() << "ms; vertical merges... " << flush;
			compact(false); // actually, always rebuilding seems faster than moving, probably because of subsequent cache efficiency
			verticalMerges(iteration, mergeCallback);
			if (verbose) cout << timer.getAndReset() << " ms; " << summary() << endl;
			iteration++;
			checkConsistency();
		}
		if (verbose) cout << summary() << endl;
	}

	void horizontalMerges(const int iteration, const std::function<void (const int, const int, const int, const MergeType)> &mergeCallback) {
		for (int nodeId = _numNodes-1; nodeId >= 0; --nodeId) {
			const NodeType& node = nodes[nodeId];
			if (node.numEdges() < 2) {
				continue;
			}
#ifndef NDEBUG
			for (EdgeType* edge = firstEdge(nodeId); edge < lastEdge(nodeId); ++edge) {
				assert(nodes[edge->headNode].parent == nodeId);
			}
#endif
			EdgeType *leftEdge, *rightEdge;
			int left, right, newNode;
			MergeType mergeType;
			int edgeNum;
			for (edgeNum = 0; edgeNum < node.numEdges() - 1; edgeNum += 2) {
				leftEdge = firstEdge(nodeId) + edgeNum;
				rightEdge = leftEdge + 1;
				assert(leftEdge->valid && rightEdge->valid);
				left = leftEdge->headNode;
				right = rightEdge->headNode;
				if (nodes[left].isLeaf() || nodes[right].isLeaf()) {
					nodes[left].lastMergedIn = iteration;
					nodes[right].lastMergedIn = iteration;
					mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
				}
			}

			if (edgeNum == node.numEdges() - 1) {
				// the odd case
				leftEdge = lastEdge(nodeId);
				left = leftEdge->headNode;
				const NodeType& child = nodes[left];
				if (!child.isLeaf() || node.numEdges() <= 2 || !(leftEdge-1)->valid || !(leftEdge-2)->valid) {
					// not a leaf or child has at most one sibling
					continue;
				}
				const int childMinusOne = (leftEdge-1)->headNode;
				const int childMinusTwo = (leftEdge-2)->headNode;
				if (!nodes[childMinusOne].isLeaf() && !nodes[childMinusTwo].isLeaf()) {
					// child is a leaf, but its two left siblings are not
					mergeSiblings(leftEdge-1, leftEdge, newNode, mergeType);
					mergeCallback(childMinusOne, left, newNode, mergeType);
				}
			}
		}
	}

	void verticalMerges(const int iteration, const std::function<void (const int, const int, const int, const MergeType)> &mergeCallback) {
		vector<int> nodesToMerge;  // TODO is a linked list faster?
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			const NodeType& node = nodes[nodeId];
			if (node.parent >= 0 && node.numEdges() != 1 && nodes[node.parent].numEdges() == 1) {
				// only interested in nodes without siblings where the chain can't be extended further
				nodesToMerge.push_back(nodeId);
			}
		}

		/*cout << "nodes to merge:";
		for (int nodeId : nodesToMerge) cout << " " << nodeId;
		cout << endl;*/

		for (int nodeId : nodesToMerge) {
			//cout << "Checking out node " << nodeId << endl;
			int parentId = nodes[nodeId].parent;
			while (parentId >= 0 && nodes[parentId].numEdges() == 1 && nodes[parentId].parent >= 0 && nodes[parentId].lastMergedIn < iteration) {
				//cout << "node: " << nodeId << " = " << nodes[nodeId] << " parent: " << parentId << " = " << nodes[parentId] << endl;
				NodeType &node(nodes[nodeId]), &parent(nodes[parentId]);
				node.lastMergedIn = iteration;
				parent.lastMergedIn = iteration;

				MergeType mergeType;
				mergeChain(parentId, mergeType);
				//cout << "node: " << nodeId << " = " << node << " parent: " << parentId << " = " << parent << endl;
				mergeCallback(parentId, nodeId, parentId, mergeType);

				nodeId = parent.parent;
				if (nodeId >= 0) {
					parentId = nodes[nodeId].parent;
				} else {
					break;  // break while loop
				}
			}
		}
	}

	void mergeSiblings(const EdgeType* leftEdge, const EdgeType* rightEdge, int& newNode, MergeType& mergeType) {
		assert(leftEdge->valid && rightEdge->valid);
		const int leftId(leftEdge->headNode), rightId(rightEdge->headNode);
		assert(0 <= leftId < _numNodes && 0 <= rightId < _numNodes);
		NodeType &left(nodes[leftId]), &right(nodes[rightId]);
		assert(left.parent == right.parent);
		assert(left.isLeaf() || right.isLeaf());

		if (left.isLeaf() && right.isLeaf()) {
			mergeType = HORZ_NO_BBN;
		} else if (left.isLeaf()) {
			mergeType = HORZ_RIGHT_BBN;
		} else {
			mergeType = HORZ_LEFT_BBN;
		}

		if (right.isLeaf()) {
			// left is what remains
			removeEdge(right.parent, edgeId(rightEdge), false);
			right.parent = -1; // just to make sure
			//cout << "\tsetting parent of " << rightId << " to -1: " << right << endl;
			newNode = leftId;
		} else {
			removeEdge(left.parent, edgeId(leftEdge), false);
			left.parent = -1;
			//cout << "\tsetting parent of " << leftId << " to -1: " << left << endl;
			newNode = rightId;
		}
	}

	void mergeChain(const int middleId, MergeType& mergeType) {
		assert(0 <= middleId < _numNodes);
		NodeType& middle = nodes[middleId];
		assert(middle.numEdges() == 1);
		int childId = firstEdge(middleId)->headNode;
		NodeType& child = nodes[childId];
		//cout << "\tmerging chain. middle: " << middleId << " = " << middle << " child: " << childId << " = " << child << "; removing edge from " << middleId << " to " << childId << endl;
		removeEdge(middleId, middle.firstEdgeIndex);
		child.parent = -1;
		if (child.isLeaf()) {
			//cout << "\tchild is a leaf, merge type (a)" << endl;
			mergeType = VERT_NO_BBN;
		} else {
			//cout << "\tchild is not a leaf, attaching its edges to middle:" << flush;
			// attach child's children to middle
			for (EdgeType *edge = firstEdge(childId); edge <= lastEdge(childId); ++edge) {
				//cout << " edge " << *edge << flush;
				nodes[edge->headNode].parent = middleId;
			}
			middle.firstEdgeIndex = child.firstEdgeIndex;
			middle.lastEdgeIndex  = child.lastEdgeIndex;
			child.lastEdgeIndex = child.firstEdgeIndex - 1;
			//cout << " nodes are now: middle " << middleId << " = " << nodes[middleId] << " child " << childId << " = " << nodes[childId] << endl;
			mergeType = VERT_WITH_BBN;
		}
	}

	std::string summary() const {
		std::stringstream s;
		s << "Ordered tree with " << _numNodes << " nodes and " << _numEdges << " edges";
		return s.str();
	}

	std::string toString() const {
		std::stringstream os;
		os << summary() << endl << "Nodes:";
		for (uint i = 0; i < nodes.size(); ++i) {
			os << " " << nodes[i];
		}
		os << endl << "Edges:";
		for (uint i = 0; i < edges.size(); ++i) {
			os << " " << edges[i];
		}
		return os.str();
	}

	template<typename treeType>
	friend std::ostream& operator<<(std::ostream& os, const treeType &tree) {
		os << tree.summary() << endl << "Nodes:";
		for (uint i = 0; i < tree.nodes.size(); ++i) {
			os << " " << tree.nodes[i];
		}
		os << endl << "Edges:";
		for (uint i = 0; i < tree.edges.size(); ++i) {
			os << " " << tree.edges[i];
		}
		return os;
	}

	void checkConsistency() {
#ifndef NDEBUG
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			NodeType &node = nodes[nodeId];
			assert(node.lastEdgeIndex >= node.firstEdgeIndex - 1);
			for (EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
				assert(edge->headNode >= 0 && nodes[edge->headNode].parent == nodeId);
			}
		}
#endif
	}

	void compact(const bool verbose = true) {
		Timer timer;
		if (_numEdges + 1 == (int) edges.size()) {
			if (verbose) cout << "GC: nothing to do" << endl;
			return;
		}
		vector<EdgeType> newEdges;
		newEdges.reserve(_numEdges + 1);
		newEdges.push_back(edges[0]);  // dummy edge
		uint oldSize;
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			oldSize = newEdges.size();
			if (nodes[nodeId].numEdges() > 0) {
				for (EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
					if (edge->valid) {
						newEdges.push_back(*edge);
					}
				}
			}
			nodes[nodeId].firstEdgeIndex = oldSize;
			nodes[nodeId].lastEdgeIndex  = newEdges.size() - 1;
		}
		_firstFreeEdge = newEdges.size();
		edges.swap(newEdges);
		if (verbose) cout << "GC: " << timer.elapsedMillis() << "ms, edge buffer now " << edges.size() << " edges, was "
			<< newEdges.size() << " (" << (edges.size() * 100.0) / newEdges.size() << "%)" << endl;
	}

	void inplaceCompact(const bool verbose = true) {
		Timer timer;
		int count = 0;
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			NodeType &node = nodes[nodeId];
			int freeEdgeId = node.firstEdgeIndex;
			for (int edgeId = node.firstEdgeIndex; edgeId <= node.lastEdgeIndex; ++edgeId) {
				EdgeType *edge = edges.data() + edgeId;
				if (!edge->valid) continue;
				if (edgeId == freeEdgeId) {
					freeEdgeId++;
					continue;
				} else {
					edges[freeEdgeId] = edges[edgeId];
					freeEdgeId++;
					count++;
				}
			}
			for (int edgeId = freeEdgeId; edgeId <= node.lastEdgeIndex; ++edgeId) {
				edges[edgeId].valid = false;
			}
			node.lastEdgeIndex = freeEdgeId - 1;
		}
		if (verbose) cout << "Inplace compaction moved " << count << " edges (" << (count * 100.0 / _numEdges) << "%) in " << timer.elapsedMillis() << "ms" << endl;
	}

protected:
	void initialise(const int n, const int m) {
		nodes.clear();
		edges.clear();

		nodes.reserve(n);
		edges.reserve(m);

		// add a dummy edge
		edges.resize(1);
		edges[0].valid = false;

		_numNodes = 0;
		_numEdges = 0;
		_firstFreeNode = 0;
		_firstFreeEdge = 1;
	}

	EdgeType* _prepareEdge(const int edgeId, const int from, const int to) {
		_numEdges++;
		nodes[to].parent = from;
		edges[edgeId].valid = true;
		edges[edgeId].headNode = to;
		return edges.data() + edgeId;
	}


public:
	vector<NodeType> nodes;
	vector<EdgeType> edges;
	int _firstFreeNode;
	int _firstFreeEdge;
	int _numNodes;
	int _numEdges;
};
