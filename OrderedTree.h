#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include "Common.h"
#include "Timer.h"

using std::cout;
using std::endl;
using std::string;

// #define awfulness
#define FORALL_NODES(tree, node) for (int node = 0; node < tree._numNodes; ++node)
#define FORALL_EDGES(tree, node, edge)                                                                                 \
	for (int node = 0; node < tree._numNodes; ++node)                                                                  \
		for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)
#define FORALL_OUTGOING_EDGES(tree, node, edge)                                                                        \
	for (auto edge = tree.firstEdge(node); edge <= tree.lastEdge(node); ++edge)

/// Ordered tree data structure
/**
 * Holds an ordered tree
 *
 * The tree is implemented as an adjacency array
 * (See section 8.2 of http://people.mpi-inf.mpg.de/~mehlhorn/ftp/Toolbox/GraphRep.pdf)
 *
 * You can define your own node and edge types, e.g. add labels
 * to the nodes or values to the edges if you wish
 */
template <typename NodeType, typename EdgeType>
class OrderedTree {
public:
	/// the node type used in this tree
	typedef NodeType nodeType;
	/// the type of the edges used in this tree
	typedef EdgeType edgeType;

	OrderedTree(const int n = 0, const int m = 0) {
		initialise(n, m);
	}

	OrderedTree(const OrderedTree<NodeType, EdgeType> &other) :
		nodes(other.nodes),
		edges(other.edges),
		_firstFreeNode(other._firstFreeNode),
		_firstFreeEdge(other._firstFreeEdge),
		_numNodes(other._numNodes),
		_numEdges(other._numEdges) {}

	/// pointer to the dummy edge
	EdgeType *firstEdge() {
		return edges.data();
	}
	/// Pointer to a node's first edge. Does not check whether the node actually has outgoing edges.
	/// \param u a node ID (index in the node vector)
	/// \return a pointer to u's first outgoing edge
	EdgeType *firstEdge(const int u) {
		return edges.data() + nodes[u].firstEdgeIndex;
	}
	/// const pointer to a node's first edge. Does not check whether the node actually has outgoing edges.
	/// \param u a node ID (index in the node vector)
	/// \return a const pointer to u's first outgoing edge
	const EdgeType *firstEdge(const int u) const {
		return edges.data() + nodes[u].firstEdgeIndex;
	}

	/// pointer to the last edge
	EdgeType *lastEdge() {
		return edges.data() + _numEdges - 1;
	}
	/// Pointer to a node's last edge. Does not check wether the node actually has outgoing edges.
	/// \param u a node ID (index in the node vector)
	/// \return a pointer to u's last outgoing edge
	EdgeType *lastEdge(const int u) {
		return edges.data() + nodes[u].lastEdgeIndex;
	}
	/// const ointer to a node's last edge. Does not check wether the node actually has outgoing edges.
	/// \param u a node ID (index in the node vector)
	/// \return a const pointer to u's last outgoing edge
	const EdgeType *lastEdge(const int u) const {
		return edges.data() + nodes[u].lastEdgeIndex;
	}

	/// Get node ID from pointer
	/// \param node a node pointer
	/// \return the node's ID (index in the node vector)
	int nodeId(const NodeType *node) {
		return (node - nodes.data());
	}
	/// Get edge ID from pointer
	/// \param edge an edge pointer
	/// \return the edge's ID (index in the edge vector)
	int edgeId(const EdgeType *edge) {
		return (edge - edges.data());
	}

	/// Add a node to the tree
	/// \return the new node's ID
	int addNode() {
		if (_firstFreeNode <= (int)nodes.size()) {
			nodes.resize(_firstFreeNode + 1);
		}
		nodes[_firstFreeNode].firstEdgeIndex = _firstFreeEdge;
		nodes[_firstFreeNode].lastEdgeIndex = _firstFreeEdge - 1;
		++_numNodes;
		return _firstFreeNode++;
	}

	/// Add multiple nodes
	/// \param n the number of nodes to add
	/// \return the ID of the first node added
	int addNodes(const int n) {
		for (int i = 0; i < n; ++i) {
			addNode();
		}
		return _firstFreeNode - n;
	}

	/// Add an edge to the tree
	/// \param from tail (source) node ID
	/// \param to head (destination) node ID
	/// \param extraSpace extra space to allocate for more outgoing edges
	/// of 'from' if more edges need to be allocated
	EdgeType *addEdge(const int from, const int to, const int extraSpace = 0) {
		if (from == to) {
			cout << "not adding cycle from " << from << " to " << to << endl;
			return NULL;
		}
		NodeType &node = nodes[from];
		int newId = node.lastEdgeIndex + 1;
		// Check for space to the right
		if (newId < (int)edges.size() && !edges[newId].valid) {
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

	void killNodes() {
		int nodeId(_numNodes - 1);
		while (nodes[nodeId].parent < 0 && --nodeId > 0);
		_numNodes = nodeId + 1;
		_firstFreeNode = _numNodes;
	}

	/// Remove an edge from the tree
	/// \param from the edge's tail (source) node
	/// \param edge the edge's ID
	/// \param compact whether to consolidate 'from's outgoing edges
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

	/// Remove an edge between to nodes from the tree.
	/// Finds and removes the *first* edge from 'from' to 'to'
	/// \param from tail (source) node ID
	/// \param to head (destination) node ID
	/// \param compact wether to consolidate 'from's outgoing edges after removal
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

	/// Merge two descendants of the same node (i.e., siblings)
	/// \param leftEdge pointer to the edge leading to the left child
	/// \param rightEdge pointer to the edge leading to the right edge
	/// \param newNode will hold the ID of the merged node after this function returns
	/// \param mergeType will hold the type of the merge that was done after this returns
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

	/// Merge two chained edges like this: a -> b -> c will become a -> b,
	/// where b and c are the only children of their parents.
	/// Any potential children of c will be attached to b.
	/// \param middleId the middle node's ID in this merge (b in the example)
	/// \param mergeType will be set to the type of the merge performed
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

	/// Check whether this tree is equal to another tree.
	/// \param other the other tree
	/// \param labels the labels for this tree. Label type is kept very general deliberately
	/// \param otherLabels the other tree's labels
	/// \param verbose whether to print an error traceback if the trees are not equal
	template <typename LabelType>
	bool isEqual(const OrderedTree<NodeType, EdgeType> &other, LabelType &labels, LabelType &otherLabels, const bool verbose = false) const {
		if (_numNodes != other._numNodes || _numEdges != other._numEdges) {
			return false;
		}
		return nodesEqual(other, labels, otherLabels, 0, 0, verbose);
	}

	/// Recursive node comparison helper function used by isEqual(). You should not need to use this directly.
	template <typename LabelType>
	bool nodesEqual(const OrderedTree<NodeType, EdgeType> &other, LabelType &labels, LabelType &otherLabels, const int nodeId, const int otherNodeId, const bool verbose = false) const {
		const NodeType &node(nodes[nodeId]), &otherNode(other.nodes[otherNodeId]);
		if (node.numEdges() != otherNode.numEdges()) {
			if (verbose) cout << "Edge count mismatch at nodes " << nodeId << " and " << otherNodeId << " : " << node.numEdges() << " vs " << otherNode.numEdges() << endl;
			return false;
		}
		if (labels[nodeId] != otherLabels[otherNodeId]) {
			if (verbose) cout << "Label mismatch at nodes " << nodeId << " and " << otherNodeId << " : '" << labels[nodeId] << "' vs '" << otherLabels[otherNodeId] << "'" << endl;
			return false;
		}

		for (int i = 0; i < node.numEdges(); ++i) {
			const int headId(edges[node.firstEdgeIndex + i].headNode);
			const int otherHeadId(other.edges[otherNode.firstEdgeIndex + i].headNode);
			assert(nodes[headId].parent == nodeId);
			assert(other.nodes[otherHeadId].parent == otherNodeId);
			if (!nodesEqual(other, labels, otherLabels, headId, otherHeadId)) {
				if (verbose) cout << "Children no " << i+1 << " of " << node.numEdges() <<" (" << headId << " and " << otherHeadId << ", labels " << labels[headId] << " and " << otherLabels[otherHeadId] << "), parent " << nodeId << " / " << otherNodeId << " did not match, children: " << nodes[headId] << " and " << other.nodes[otherHeadId] << endl;
				return false;
			}
		}

		return true;
	}

	/// A one-line summary of the tree
	string summary() const {
		std::stringstream s;
		s << "Tree with n = " << _numNodes << " m = " << std::setw(NUM_DIGITS(_numNodes)) << _numEdges;
		return s.str();
	}

	/// A rather compact representation of the tree as a string.
	/// Comprises nodes with children or a parent, and valid edges.
	/// Format: "  ID/(node or edge)"
	string shortString() const {
		std::stringstream os;
		os << summary() << endl << "Nodes:";
		for (uint i = 0; i < nodes.size(); ++i) {
			if (nodes[i].hasChildren() || nodes[i].parent != -1)
				os << "  " << i << "/" << nodes[i];
		}
		os << endl << "Edges:";
		for (uint i = 0; i < edges.size(); ++i) {
			if (edges[i].valid)
				os << "  " << i << "/" << edges[i];
		}
		return os.str();
	}

	/// String representation of the tree, including all nodes and edges
	string toString() const {
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

	friend std::ostream &operator<<(std::ostream &os, const OrderedTree<NodeType, EdgeType> &tree) {
		os << tree.summary() << endl << "Nodes:";
		for (uint i = 0; i < tree.nodes.size(); ++i) {
			os << "  " << i << "/" << tree.nodes[i];
		}
		os << endl << "Edges:";
		for (uint i = 0; i < tree.edges.size(); ++i) {
			os << "  " << i << "/" << tree.edges[i];
		}
		return os;
	}

	/// Perform a few consistency checks.
	/// NOP if NDEBUG is set
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

	/// Compress the edge vector, removing gaps
	/// \param verbose whether to print some debug information
	/// \param factor how many times the number of its outgoing edges a node's
	/// edge space shall be allocated. If you set this to 2, for example, space
	/// for another edge will be reserved for each edge that there is, so that
	/// inserting an edge does not cause moving or reallocation.
	void compact(const bool verbose = true, const int factor = 1) {
		Timer timer;
		if (_numEdges + 1 == (int)edges.size()) {
			if (verbose) cout << "GC: nothing to do" << endl;
			return;
		}
		std::vector<EdgeType> newEdges;
		// Guess the amount of space needed for extra empty edges
		const int numEdgesReserved(_numEdges * factor + 1);
		if (verbose)
			cout << "GC: allocating " << numEdgesReserved << " edges (" << numEdgesReserved * sizeof(EdgeType) / 1e6
				 << "MB); " << std::flush;
		newEdges.reserve(numEdgesReserved);
		newEdges.push_back(edges[0]); // dummy edge
		int oldSize(1);
		// Copy each node's valid edges to the new array
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			// While a bit counterintuitive at first, this check speeds things up because now
			// we don't need to fetch two edges just to determine if we need to copy anything
			const bool hasChildren = nodes[nodeId].hasChildren();
			if (hasChildren) {
				for (EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
					if (edge->valid) {
						newEdges.push_back(*edge);
					}
				}
			}
			nodes[nodeId].firstEdgeIndex = oldSize;
			oldSize = (int)newEdges.size();
			nodes[nodeId].lastEdgeIndex = oldSize - 1;
			if (hasChildren)  {
				oldSize += nodes[nodeId].numEdges() * (factor - 1);
				newEdges.resize(oldSize);
			}
		}
		_firstFreeEdge = newEdges.size();
		edges.swap(newEdges);

		if (verbose)
			cout << timer.get() << "ms, " << edges.size() << " / " << newEdges.size() << " edges left ("
				 << (edges.size() * 100.0) / newEdges.size() << "%)" << endl;
	}

	void compactNode(const int nodeId) {
		NodeType &node = nodes[nodeId];
		int freeEdgeId = node.firstEdgeIndex;
		for (int edgeId = node.firstEdgeIndex; edgeId <= node.lastEdgeIndex; ++edgeId) {
			EdgeType *edge = edges.data() + edgeId;
			if (!edge->valid) continue;
			if (edgeId != freeEdgeId) {
				assert(freeEdgeId < edgeId);
				// edges are trivially copyable
				std::memcpy(edges.data() + freeEdgeId, edge, sizeof(EdgeType));
				edge->valid = false;
			}
			freeEdgeId++;
		}
		for (int edgeId = freeEdgeId; edgeId <= node.lastEdgeIndex; ++edgeId) {
			edges[edgeId].valid = false;
		}
		node.lastEdgeIndex = freeEdgeId - 1;
	}

	/// Do an inplace compaction of each node's vertices
	/// This is faster than rebuilding compaction.
	void inplaceCompact(const bool verbose = true) {
		Timer timer;
		int count = 0;
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			NodeType &node = nodes[nodeId];
			// While maybe a bit counterintuitive at first, this check speeds thing up because
			// we don't need to do all the other more expensive checks for nodes without children
			if (!node.hasChildren()) continue;
			int freeEdgeId = node.firstEdgeIndex;
			for (int edgeId = node.firstEdgeIndex; edgeId <= node.lastEdgeIndex; ++edgeId) {
				EdgeType *edge = edges.data() + edgeId;
				if (!edge->valid) continue;
				if (edgeId != freeEdgeId) {
					// edges are trivially copyable
					std::memcpy(edges.data() + freeEdgeId, edge, sizeof(EdgeType));
					edge->valid = false;
					count++;
				}
				freeEdgeId++;
			}
			for (int edgeId = freeEdgeId; edgeId <= node.lastEdgeIndex; ++edgeId) {
				edges[edgeId].valid = false;
			}
			node.lastEdgeIndex = freeEdgeId - 1;
		}
		if (verbose)
			cout << "Inplace compaction moved " << count << " edges (" << (count * 100.0 / _numEdges) << "%) in "
				 << timer.get() << "ms" << endl;
	}

	/// Do an inplace compaction of only the dirty vertices
	/// This is faster than rebuilding compaction.
	void inplaceCompact(std::vector<bool> &dirty, const bool verbose = true) {
		Timer timer;
		int count = 0;
		for (int nodeId = 0; nodeId < _numNodes; ++nodeId) {
			if (likely(!dirty[nodeId])) continue;
			NodeType &node = nodes[nodeId];
			// While maybe a bit counterintuitive at first, this check speeds thing up because
			// we don't need to do all the other more expensive checks for nodes without children
			if (!node.hasChildren()) continue;
			int freeEdgeId = node.firstEdgeIndex;
			for (int edgeId = node.firstEdgeIndex; edgeId <= node.lastEdgeIndex; ++edgeId) {
				EdgeType *edge = edges.data() + edgeId;
				if (!edge->valid) continue;
				if (edgeId != freeEdgeId) {
					// edges are trivially copyable
					std::memcpy(edges.data() + freeEdgeId, edge, sizeof(EdgeType));
					edge->valid = false;
					count++;
				}
				freeEdgeId++;
			}
			for (int edgeId = freeEdgeId; edgeId <= node.lastEdgeIndex; ++edgeId) {
				edges[edgeId].valid = false;
			}
			node.lastEdgeIndex = freeEdgeId - 1;
		}
		if (verbose)
			cout << "Inplace compaction moved " << count << " edges (" << (count * 100.0 / _numEdges) << "%) in "
				 << timer.get() << "ms" << endl;
	}

	// for statistics, mainly

	/// Perform a left fold over each node's children in post-order
	/// \param callback a callback to be called for each node with the result of the last fold operation
	/// \param fold the fold function. Parameters: previous value, current value
	/// \param initial inital value for the first folding
	template <typename T, typename Fold, typename Callback>
	const T foldLeftPostOrder(const Callback &callback, const Fold &fold, const T initial) const {
		return traverseFoldLeftPostOrder(0, callback, fold, initial);
	}

	/// This does the work for foldLeftPostOrder() and should not be used directly
	template <typename T, typename Fold, typename Callback>
	const T traverseFoldLeftPostOrder(const int nodeId, const Callback &callback,
									  const Fold &fold, const T initial) const {
		assert(0 <= nodeId && nodeId < _numNodes);
		T last(initial);
		for (const EdgeType *edge = firstEdge(nodeId); edge <= lastEdge(nodeId); ++edge) {
			last = fold(last, traverseFoldLeftPostOrder(edge->headNode, callback, fold, initial));
		}
		return callback(last);
	}

	/// Calculate the height of the tree (i.e., the maximum depth of a node).
	/// Uses foldLeftPostOrder() and worth looking at as a simple example of a fold
	int height() const {
		return foldLeftPostOrder<int>(
			[](const int depth) { return depth + 1; },
			[](const int p1, const int p2) { return std::max(p1, p2); }, 0);
	}

	/// Calculate the average depth of the nodes in the tree.
	/// Uses foldLeftPostOrder() and worth looking at as slightly more complex example for a fold
	double avgDepth() const {
		typedef std::pair<uint_fast32_t, uint_fast64_t> P;
		P countAndSum = foldLeftPostOrder<P>(
			[](const P countAndSum) { return P(countAndSum.first + 1, countAndSum.second + countAndSum.first + 1); },
			[](const P p1, const P p2) { return P(p1.first + p2.first, p1.second + p2.second); }, P(1, 0));
		return (double)countAndSum.second / countAndSum.first;
	}

	void clear() {
		initialise(0, 0);
	}

protected:
	/// Initialise the tree
	/// \param n number of nodes to reserve space for
	/// \param m number of edges to reserve space for
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

	/// Helper method for inserting new edges
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
	std::vector<NodeType> nodes;
	std::vector<EdgeType> edges;
	int _firstFreeNode;
	int _firstFreeEdge;
	int _numNodes;
	int _numEdges;
};
