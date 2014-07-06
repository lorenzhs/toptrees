#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "Common.h"
#include "Timer.h"

using std::cout;
using std::endl;
using std::function;
using std::flush;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

// #define awfulness
#define FORALL_NODES(tree, node) for (int node = 0; node < tree._numNodes; ++node)
#define FORALL_EDGES(tree, node, edge) for (int node = 0; node < tree._numNodes; ++node) for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)
#define FORALL_OUTGOING_EDGES(tree, node, edge) for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)

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

	EdgeType* addEdge(const int from, const int to, const int extraSpace = 0) {
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
		NodeType& node = nodes[from];
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

	// find the first edge from node `from` to node `to` and remove it
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

	// do iterated merges to construct a top tree
	// the callback is called for every pair of merged nodes (clusters)
	// its arguments are the ids of the two merged nodes and the new id
	// (usually one of the two old ones) as well as the type of the merge
	void doMerges(const function<void (const int, const int, const int, const MergeType)> &mergeCallback, const bool verbose = true) {
		int iteration = 0;
		Timer timer;
		const std::streamsize precision = cout.precision();
		cout <<  std::fixed << std::setprecision(1);
		while (_numEdges > 1) {
			if (verbose) cout << "It. " << std::setw(2) << iteration << ": merging horz… " << flush;

			horizontalMerges(iteration, mergeCallback);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; gc… " << flush;

			// We need to compact here because the horizontal merges don't but
			// the vertical merges need correct edge counts, so this is important!
			// And it turns out that rebuild compation is faster than inplace,
			// maybe because of subsequent cache efficiency?
			compact(false);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; vert… " << flush;

			verticalMerges(iteration, mergeCallback);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << " ms; " << summary() << endl;

			iteration++;
			checkConsistency();
		}
		// reset the output stream
		cout.unsetf(std::ios_base::fixed);
		cout << std::setprecision(precision);
		if (verbose) cout << summary() << endl;
	}

	// do one iteration of horizontal merges (step 1)
	void horizontalMerges(const int iteration, const function<void (const int, const int, const int, const MergeType)> &mergeCallback) {
		for (int nodeId = _numNodes-1; nodeId >= 0; --nodeId) {
			const NodeType& node = nodes[nodeId];
			// merging children only make sense for nodes with ≥ 2 children
			if (node.numEdges() < 2) {
				continue;
			}
#ifndef NDEBUG
			// verify that these edges indeed do belong to whoever claims to be their parent
			for (EdgeType* edge = firstEdge(nodeId); edge < lastEdge(nodeId); ++edge) {
				assert(nodes[edge->headNode].parent == nodeId);
			}
#endif
			EdgeType *leftEdge, *rightEdge, *baseEdge(firstEdge(nodeId));
			int left, right, newNode, edgeNum, numEdges(node.numEdges());
			MergeType mergeType;
			// iterate over pairs of children by index
			for (edgeNum = 0; edgeNum < (numEdges - 1); edgeNum += 2) {
				leftEdge = baseEdge + edgeNum;
				rightEdge = leftEdge + 1;
				assert(leftEdge->valid && rightEdge->valid);
				left = leftEdge->headNode;
				right = rightEdge->headNode;
				// We can only merge if at least one of the two is a leaf
				if (nodes[left].isLeaf() || nodes[right].isLeaf()) {
					nodes[left].lastMergedIn = iteration;
					nodes[right].lastMergedIn = iteration;
					mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
				}
			}

			if (edgeNum == numEdges - 1) {
				// the node has an odd number of children. check if the conditions for an odd
				// merge at the end are satisfied. We need the last child to be a leaf and the
				// two previous children to be non-leaves. This implies that they have not been
				// merged in this iteration so far (because neither is a leaf)
				leftEdge = lastEdge(nodeId);
				left = leftEdge->headNode;
				const NodeType& child = nodes[left];
				if (!child.isLeaf() || node.numEdges() <= 2 || !(leftEdge-1)->valid || !(leftEdge-2)->valid) {
					continue;
				}
				const int childMinusOne = (leftEdge-1)->headNode;
				const int childMinusTwo = (leftEdge-2)->headNode;
				if (!nodes[childMinusOne].isLeaf() && !nodes[childMinusTwo].isLeaf()) {
					// Everything is go for a merge in the "odd case"
					nodes[left].lastMergedIn = iteration;
					nodes[childMinusOne].lastMergedIn = iteration;
					mergeSiblings(leftEdge-1, leftEdge, newNode, mergeType);
					mergeCallback(childMinusOne, left, newNode, mergeType);
				}
			}
		}
	}

	// Perform an iteration of vertical (chain) merges.
	void verticalMerges(const int iteration, const function<void (const int, const int, const int, const MergeType)> &mergeCallback) {
		// First, we collect all the vertices from which a merge chain can originate upwards
		// This is needed to prevent repeated merges of the same chain in one iteration
		// I guess we could do this with the .lastMergedIn attribute as well? XXX TODO
		vector<int> nodesToMerge;
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			const NodeType& node = nodes[nodeId];
			if (node.parent >= 0 && !node.hasOnlyOneChild() && nodes[node.parent].hasOnlyOneChild()) {
				// only interested in nodes without siblings where the chain can't be extended further
				nodesToMerge.push_back(nodeId);
			}
		}

		for (int nodeId : nodesToMerge) {
			int parentId = nodes[nodeId].parent;
			// Follow the chain upwards until we hit a node where it has to end. Possible cases:
			// a) node or parent is the root node
			// b) parent has more than one child
			// otherwise, merge the chain grandparent -> parent -> node
			while (parentId >= 0 && nodes[parentId].hasOnlyOneChild() && nodes[parentId].parent >= 0 &&
				nodes[parentId].lastMergedIn < iteration && nodes[nodes[parentId].parent].hasOnlyOneChild()) {
				//cout << "merging " << parentId << " with its child " << nodeId << endl;
				NodeType &node(nodes[nodeId]), &parent(nodes[parentId]);
				node.lastMergedIn = iteration;
				parent.lastMergedIn = iteration;

				MergeType mergeType;
				mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);

				// Follow the chain upwards if possible
				nodeId = parent.parent;
				if (nodeId >= 0) {
					parentId = nodes[nodeId].parent;
				} else {
					break;  // break while loop
				}
			}
			/*cout << "merging loop ended with nodeId = " << nodeId << " parentId = " << parentId;
			if (nodeId >= 0) cout << " node = " << nodes[nodeId];
			if (parentId >= 0) cout << " parent = " << nodes[parentId];
			cout << endl;*/

			if (nodeId >= 0 && parentId >= 0 && nodes[parentId].hasOnlyOneChild() && nodes[parentId].lastMergedIn < iteration && nodes[parentId].parent >= 0) {
				// We hit the "odd case"
				//cout << "ODD CASE: nodeId = " << nodeId << " " << nodes[nodeId] << " parentId = "<< parentId << " " << nodes[parentId] << endl;
				assert(!nodes[nodes[parentId].parent].hasOnlyOneChild());
				nodes[nodeId].lastMergedIn = iteration;
				nodes[parentId].lastMergedIn = iteration;
				MergeType mergeType;
				mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);
			}
		}
	}

	// Merge two descendants of the same node (i.e., siblings)
	// Will *set* the newNode and mergeType parameters
	void mergeSiblings(const EdgeType* leftEdge, const EdgeType* rightEdge, int& newNode, MergeType& mergeType) {
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
	void mergeChain(const int middleId, MergeType& mergeType) {
		// Retrieve nodes and perform sanity checks
		assert(0 <= middleId && middleId < _numNodes);
		NodeType& middle = nodes[middleId];
		assert(middle.hasOnlyOneChild());
		int childId = firstEdge(middleId)->headNode;
		NodeType& child = nodes[childId];

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
			middle.lastEdgeIndex  = child.lastEdgeIndex;
			child.lastEdgeIndex = child.firstEdgeIndex - 1;

			mergeType = VERT_WITH_BBN;
		}
	}

	string summary() const {
		stringstream s;
		s << "Tree with n = " << _numNodes << " m = " << std::setw(NUM_DIGITS(_numNodes)) << _numEdges;
		return s.str();
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

	friend std::ostream& operator<<(std::ostream& os, const OrderedTree<NodeType, EdgeType> &tree) {
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
		if (_numEdges + 1 == (int) edges.size()) {
			if (verbose) cout << "GC: nothing to do" << endl;
			return;
		}
		vector<EdgeType> newEdges;
		// Guess the amount of space needed for extra empty edges
		const int numEdgesReserved(_numEdges*factor + 1);
		if (verbose)
			cout << "GC: allocating " << numEdgesReserved << " edges (" << numEdgesReserved*sizeof(EdgeType)/1e6 << "MB); " << flush;
		newEdges.reserve(numEdgesReserved);
		newEdges.push_back(edges[0]);  // dummy edge
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
			nodes[nodeId].lastEdgeIndex  = newEdges.size() - 1;
			if (nodes[nodeId].hasChildren())
				newEdges.resize(newEdges.size() + nodes[nodeId].numEdges() * (factor - 1));
		}
		_firstFreeEdge = newEdges.size();
		edges.swap(newEdges);

		if (verbose) cout << timer.elapsedMillis() << "ms, " << edges.size() << " / "
			<< newEdges.size() << " edges left (" << (edges.size() * 100.0) / newEdges.size() << "%)" << endl;
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

	// Helper method for inserting new edges in order to remove repetition
	EdgeType* _prepareEdge(const int edgeId, const int from, const int to) {
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
