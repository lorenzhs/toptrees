#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include "Common.h"
#include "Timer.h"

using std::cout;
using std::endl;
using std::function;
using std::flush;
using std::ostream;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

// #define awfulness
#define FORALL_NODES(tree, node) for (int node = 0; node < tree._numNodes; ++node)
#define FORALL_EDGES(tree, node, edge)                                                                                 \
	for (int node = 0; node < tree._numNodes; ++node)                                                                  \
		for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)
#define FORALL_OUTGOING_EDGES(tree, node, edge)                                                                        \
	for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)

template <typename NodeType, typename EdgeType>
class OrderedTree {
public:
	typedef NodeType nodeType;
	typedef EdgeType edgeType;

	OrderedTree(const int n = 0, const int m = 0) {
		initialise(n, m);
	}

	EdgeType *firstEdge() {
		return edges.data();
	}
	EdgeType *firstEdge(const int u) {
		return edges.data() + nodes[u].firstEdgeIndex;
	}
	const EdgeType *firstEdge(const int u) const {
		return edges.data() + nodes[u].firstEdgeIndex;
	}

	EdgeType *lastEdge() {
		return edges.data() + _numEdges - 1;
	}
	EdgeType *lastEdge(const int u) {
		return edges.data() + nodes[u].lastEdgeIndex;
	}
	const EdgeType *lastEdge(const int u) const {
		return edges.data() + nodes[u].lastEdgeIndex;
	}

	// Get ID from pointer, woohoo for not doing this in Scala!
	int nodeId(const NodeType *node) {
		return (node - nodes.data());
	}
	int edgeId(const EdgeType *edge) {
		return (edge - edges.data());
	}

	// Add a node and return its id
	int addNode() {
		if (_firstFreeNode <= (int)nodes.size()) {
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
		for (EdgeType *edge = firstEdge(u); edge <= lastEdge(u); ++edge) {
			edge->valid = false;
		}
		nodes[u].lastEdgeIndex = nodes[u].firstEdgeIndex - 1;
	}

	EdgeType *addEdge(const int from, const int to, const int extraSpace = 0) {
		NodeType &node = nodes[from];
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
				edges[i] = edges[i + 1];
			}
			return _prepareEdge(node.lastEdgeIndex, from, to);
		}

		if (node.numEdges() >= (int)edges.size() - _firstFreeEdge) {
			// allocate more space. this allocates too much if the node is at the end but we don't really care
			edges.resize(_firstFreeEdge + node.numEdges() + 1);
		}

		if (node.lastEdgeIndex == _firstFreeEdge - 1) {
			// We're already at the end
			newId = _firstFreeEdge;
		} else {
			edges.resize(edges.size() + extraSpace);
			// move node's edges to the end
			_firstFreeEdge += extraSpace;
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
		NodeType &node = nodes[from];
		// check if we can just move the boundaries of the node's edge space
		if (edge == node.lastEdgeIndex) {
			node.lastEdgeIndex--;
		} else if (edge == node.firstEdgeIndex) {
			node.firstEdgeIndex++;
		} else if (compact) {
			// because this is an ordered tree, we have to move edges around :(
			if ((edge - node.firstEdgeIndex) >= (node.lastEdgeIndex - edge)) {
				// there are morge edges on the left => move right side to the left
				for (int i = edge; i < node.lastEdgeIndex; ++i) {
					edges[i] = edges[i + 1];
				}
				edges[node.lastEdgeIndex--].valid = false;
			} else {
				// move edges on the left side to the right
				for (int i = edge; i > node.firstEdgeIndex; --i) {
					edges[i] = edges[i - 1];
				}
				edges[node.firstEdgeIndex++].valid = false;
			}
		}
		_numEdges--;
	}

	// find the first edge from node `from` to node `to` and remove it
	void removeEdgeTo(const int from, const int to, const bool compact = true) {
		NodeType &node = nodes[from];
		for (int i = node.firstEdgeIndex; i <= node.lastEdgeIndex; ++i) {
			if (edges[i].headNode == to) {
				assert(nodes[edges[i].headNode].parent == from);
				removeEdge(from, i, compact);
				return;
			}
		}
	}

	// Merge two descendants of the same node (i.e., siblings)
	// Will *set* the newNode and mergeType parameters
	void mergeSiblings(const EdgeType *leftEdge, const EdgeType *rightEdge, int &newNode, MergeType &mergeType) {
		// retrieve nodes and perform sanity checks
		assert(leftEdge->valid && rightEdge->valid);
		const int leftId(leftEdge->headNode), rightId(rightEdge->headNode);
		assert(0 <= leftId && leftId < _numNodes && 0 <= rightId && rightId < _numNodes);
		NodeType &left(nodes[leftId]), &right(nodes[rightId]);
		assert(left.parent == right.parent);
		assert(left.isLeaf() || right.isLeaf());

		// Determine the type of the merge
		if (left.isLeaf() && right.isLeaf()) {
			mergeType = HORZ_NO_BBN;
		} else if (left.isLeaf()) {
			mergeType = HORZ_RIGHT_BBN;
		} else {
			mergeType = HORZ_LEFT_BBN;
		}

		if (right.isLeaf()) {
			// We can kill the right node and keep the (potential) children of the left one
			removeEdge(right.parent, edgeId(rightEdge), false);
			right.parent = -1; // this makes things a lot easier and faster in the iterations
			newNode = leftId;
		} else {
			// The right node is not a leaf, so the left one has to be. We can safely
			// kill it and keep only the right node.
			removeEdge(left.parent, edgeId(leftEdge), false);
			left.parent = -1;
			newNode = rightId;
		}
	}

	// Merge two chained edges like this: a -> b -> c will become a -> b
	// where b and c are the only children of their parents
	// Any potential children of c will be attached to b
	// The parameter `middleId` is the ID of node b in this example
	void mergeChain(const int middleId, MergeType &mergeType) {
		// Retrieve nodes and perform sanity checks
		assert(0 <= middleId && middleId < _numNodes);
		NodeType &middle = nodes[middleId];
		assert(middle.hasOnlyOneChild());
		int childId = firstEdge(middleId)->headNode;
		NodeType &child = nodes[childId];

		// Cut off the child. As middle has only one, its first edge goes to its child.
		removeEdge(middleId, middle.firstEdgeIndex);
		child.parent = -1;

		if (child.isLeaf()) {
			mergeType = VERT_NO_BBN;
		} else {
			// Attach child's children to middle. This requires setting the
			// parent pointers and the nodes' edge indices
			for (EdgeType *edge = firstEdge(childId); edge <= lastEdge(childId); ++edge) {
				nodes[edge->headNode].parent = middleId;
			}
			middle.firstEdgeIndex = child.firstEdgeIndex;
			middle.lastEdgeIndex = child.lastEdgeIndex;
			child.lastEdgeIndex = child.firstEdgeIndex - 1;

			mergeType = VERT_WITH_BBN;
		}
	}

	string summary() const {
		stringstream s;
		s << "Tree with n = " << _numNodes << " m = " << std::setw(NUM_DIGITS(_numNodes)) << _numEdges;
		return s.str();
	}

	string shortString() const {
		stringstream os;
		os << summary() << endl << "Nodes:";
		for (uint i = 0; i < nodes.size(); ++i) {
			if (nodes[i].hasChildren())
				os << " " << i << "/" << nodes[i];
		}
		os << endl << "Edges:";
		for (uint i = 0; i < edges.size(); ++i) {
			os << " " << edges[i];
		}
		return os.str();
	}

	string toString() const {
		stringstream os;
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

	friend std::ostream &operator<<(std::ostream &os, const OrderedTree<NodeType, EdgeType> &tree) {
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

	void compact(const bool verbose = true, const int factor = 1) {
		Timer timer;
		if (_numEdges + 1 == (int)edges.size()) {
			if (verbose) cout << "GC: nothing to do" << endl;
			return;
		}
		vector<EdgeType> newEdges;
		// Guess the amount of space needed for extra empty edges
		const int numEdgesReserved(_numEdges * factor + 1);
		if (verbose)
			cout << "GC: allocating " << numEdgesReserved << " edges (" << numEdgesReserved * sizeof(EdgeType) / 1e6
				 << "MB); " << flush;
		newEdges.reserve(numEdgesReserved);
		newEdges.push_back(edges[0]); // dummy edge
		uint oldSize;
		// Copy each node's valid edges to the new array
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			oldSize = newEdges.size();
			// While a bit counterintuitive at first, this check speeds things up because now
			// we don't need to fetch two edges just to determine if we need to copy anything
			if (nodes[nodeId].hasChildren()) {
				for (EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
					if (edge->valid) {
						newEdges.push_back(*edge);
					}
				}
			}
			nodes[nodeId].firstEdgeIndex = oldSize;
			nodes[nodeId].lastEdgeIndex = newEdges.size() - 1;
			if (nodes[nodeId].hasChildren()) newEdges.resize(newEdges.size() + nodes[nodeId].numEdges() * (factor - 1));
		}
		_firstFreeEdge = newEdges.size();
		edges.swap(newEdges);

		if (verbose)
			cout << timer.elapsedMillis() << "ms, " << edges.size() << " / " << newEdges.size() << " edges left ("
				 << (edges.size() * 100.0) / newEdges.size() << "%)" << endl;
	}

	// Do an inplace compaction of each node's vertices
	// Seems rather slow so consider removing this in the future
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
		if (verbose)
			cout << "Inplace compaction moved " << count << " edges (" << (count * 100.0 / _numEdges) << "%) in "
				 << timer.elapsedMillis() << "ms" << endl;
	}

	// for statistics, mainly
	template <typename T>
	const T foldLeftPostOrder(const function<const T(const T)> &callback,
							  const function<const T(const T, const T)> &fold, const T initial) const {
		return traverseFoldLeftPostOrder(0, callback, fold, initial);
	}

	template <typename T>
	const T traverseFoldLeftPostOrder(const int nodeId, const function<const T(const T)> &callback,
									  const function<const T(const T, const T)> &fold, const T initial) const {
		assert(0 <= nodeId && nodeId < _numNodes);
		T last(initial);
		for (const EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
			last = fold(last, traverseFoldLeftPostOrder(edge->headNode, callback, fold, initial));
		}
		return callback(last);
	}

	int height() const {
		return foldLeftPostOrder<int>(
			[](const int depth) { return depth + 1; },
			[](const int p1, const int p2) { return std::max(p1, p2); }, 0);
	}

	double avgDepth() const {
		typedef pair<int, int> P;
		P countAndSum = foldLeftPostOrder<P>(
			[](const P countAndSum) { return P(countAndSum.first + 1, countAndSum.second + countAndSum.first); },
			[](const P p1, const P p2) { return P(p1.first + p2.first, p1.second + p2.second); }, P(1, 1));
		return (countAndSum.second * 1.0) / countAndSum.first;
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

	// Helper method for inserting new edges in order to remove repetition
	EdgeType *_prepareEdge(const int edgeId, const int from, const int to) {
		_numEdges++;
		nodes[to].parent = from;
		edges[edgeId].valid = true;
		edges[edgeId].headNode = to;
		return edges.data() + edgeId;
	}

	// just make all this stuff public, I know what I'm doing [TM]
	// ...said everyone in history who then promptly proceeded
	// to shoot themselves in the food catastrophically
public:
	vector<NodeType> nodes;
	vector<EdgeType> edges;
	int _firstFreeNode;
	int _firstFreeEdge;
	int _numNodes;
	int _numEdges;
};
