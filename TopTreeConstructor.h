#pragma once

#include <cassert>
#include <iostream>
#include <iomanip>
#include <functional>
#include <vector>

#include "Common.h"
#include "Timer.h"
#include "TopTree.h"
#include "Statistics.h"

using std::cout;
using std::endl;
using std::flush;
using std::function;
using std::vector;

template <typename TreeType, typename DataType>
class TopTreeConstructor {
	typedef typename TreeType::nodeType NodeType;
	typedef typename TreeType::edgeType EdgeType;

public:
	TopTreeConstructor(TreeType &tree, TopTree<DataType> &topTree) : tree(tree), topTree(topTree) {}

	void construct(DebugInfo *debugInfo = NULL, const bool verbose = true, const bool extraVerbose = false) {
		vector<int> nodeIds(tree._numNodes);
		for (int i = 0; i < tree._numNodes; ++i) {
			nodeIds[i] = i;
		}

		doMerges([&](const int u, const int v, const int n,
					 const MergeType type) { nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type); },
				 debugInfo, verbose, extraVerbose);
	}

protected:
	// do iterated merges to construct a top tree
	// the callback is called for every pair of merged nodes (clusters)
	// its arguments are the ids of the two merged nodes and the new id
	// (usually one of the two old ones) as well as the type of the merge
	void doMerges(const function<void(const int, const int, const int, const MergeType)> &mergeCallback,
				  DebugInfo *debugInfo, const bool verbose, const bool extraVerbose) {
		int iteration = 0;
		Timer timer;
		const std::streamsize precision = cout.precision();
		cout << std::fixed << std::setprecision(1);
		while (tree._numEdges > 1) {
			if (verbose) cout << "It. " << std::setw(2) << iteration << ": merging horz… " << flush;

			if (extraVerbose) cout << endl << tree.shortString() << endl;

			int oldNumEdges = tree._numEdges;
			horizontalMerges(iteration, mergeCallback);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; gc… " << flush;

			// We need to compact here because the horizontal merges don't but
			// the vertical merges need correct edge counts, so this is important!
			// And it turns out that rebuild compation is faster than inplace,
			// maybe because of subsequent cache efficiency?
			tree.compact(false);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; vert… " << flush;

			verticalMerges(iteration, mergeCallback);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << " ms; " << tree.summary();

			double ratio = (oldNumEdges * 1.0) / tree._numEdges;
			if (verbose && ratio < 1.25) cout << " ratio " << std::setprecision(5) << ratio << std::setprecision(1) << std::endl << tree.shortString();
			if (verbose) cout << std::endl;

			if (debugInfo != NULL)
				debugInfo->addEdgeRatio(ratio);
			iteration++;
			tree.checkConsistency();
		}
		mergeCallback(0, tree.edges[tree.nodes[0].firstEdgeIndex].headNode, 0, VERT_WITH_BBN);
		// reset the output stream
		cout.unsetf(std::ios_base::fixed);
		cout << std::setprecision(precision);
		if (verbose) cout << tree.summary() << endl;
	}

	// do one iteration of horizontal merges (step 1)
	void horizontalMerges(const int iteration,
						  const function<void(const int, const int, const int, const MergeType)> &mergeCallback) {
		for (int nodeId = tree._numNodes - 1; nodeId >= 0; --nodeId) {
			const NodeType &node = tree.nodes[nodeId];
			// merging children only make sense for nodes with ≥ 2 children
			if (node.numEdges() < 2) {
				continue;
			}
#ifndef NDEBUG
			// verify that these edges indeed do belong to whoever claims to be their parent
			for (EdgeType *edge = tree.firstEdge(nodeId); edge < tree.lastEdge(nodeId); ++edge) {
				assert(tree.nodes[edge->headNode].parent == nodeId);
			}
#endif
			EdgeType *leftEdge, *rightEdge, *baseEdge(tree.firstEdge(nodeId));
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
				if (tree.nodes[left].isLeaf() || tree.nodes[right].isLeaf()) {
					tree.nodes[left].lastMergedIn = iteration;
					tree.nodes[right].lastMergedIn = iteration;
					tree.mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
				}
			}

			if (edgeNum == numEdges - 1) {
				// the node has an odd number of children. check if the conditions for an odd
				// merge at the end are satisfied. We need the last child to be a leaf and the
				// two previous children to be non-leaves. This implies that they have not been
				// merged in this iteration so far (because neither is a leaf)
				leftEdge = tree.lastEdge(nodeId);
				left = leftEdge->headNode;
				const NodeType &child = tree.nodes[left];
				if (!child.isLeaf() || node.numEdges() <= 2 || !(leftEdge - 1)->valid || !(leftEdge - 2)->valid) {
					continue;
				}
				const int childMinusOne = (leftEdge - 1)->headNode;
				const int childMinusTwo = (leftEdge - 2)->headNode;
				if (!tree.nodes[childMinusOne].isLeaf() && !tree.nodes[childMinusTwo].isLeaf()) {
					// Everything is go for a merge in the "odd case"
					tree.nodes[left].lastMergedIn = iteration;
					tree.nodes[childMinusOne].lastMergedIn = iteration;
					tree.mergeSiblings(leftEdge - 1, leftEdge, newNode, mergeType);
					mergeCallback(childMinusOne, left, newNode, mergeType);
				}
			}
		}
	}

	// Perform an iteration of vertical (chain) merges.
	void verticalMerges(const int iteration,
						const function<void(const int, const int, const int, const MergeType)> &mergeCallback) {
		// First, we collect all the vertices from which a merge chain can originate upwards
		// This is needed to prevent repeated merges of the same chain in one iteration
		// I guess we could do this with the .lastMergedIn attribute as well? XXX TODO
		vector<int> nodesToMerge;
		for (int nodeId = 0; nodeId < tree._numNodes; ++nodeId) {
			const NodeType &node = tree.nodes[nodeId];
			if (node.parent >= 0 && !node.hasOnlyOneChild() && tree.nodes[node.parent].hasOnlyOneChild()) {
				// only interested in nodes without siblings where the chain can't be extended further
				nodesToMerge.push_back(nodeId);
			}
		}

		for (int nodeId : nodesToMerge) {
			int parentId = tree.nodes[nodeId].parent;
			// Follow the chain upwards until we hit a node where it has to end. Possible cases:
			// a) node or parent is the root node
			// b) parent has more than one child
			// otherwise, merge the chain grandparent -> parent -> node
			while (parentId >= 0 && tree.nodes[parentId].hasOnlyOneChild() && tree.nodes[parentId].parent >= 0 &&
				   tree.nodes[parentId].lastMergedIn < iteration &&
				   tree.nodes[tree.nodes[parentId].parent].hasOnlyOneChild()) {
				NodeType &node(tree.nodes[nodeId]), &parent(tree.nodes[parentId]);
				node.lastMergedIn = iteration;
				parent.lastMergedIn = iteration;

				MergeType mergeType;
				tree.mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);

				// Follow the chain upwards if possible
				nodeId = parent.parent;
				if (nodeId >= 0) {
					parentId = tree.nodes[nodeId].parent;
				} else {
					break; // break while loop
				}
			}

			if (nodeId >= 0 && parentId >= 0 && tree.nodes[parentId].hasOnlyOneChild() &&
				tree.nodes[parentId].lastMergedIn < iteration && tree.nodes[parentId].parent >= 0) {
				// We hit the "odd case"
				assert(!tree.nodes[tree.nodes[parentId].parent].hasOnlyOneChild());
				tree.nodes[nodeId].lastMergedIn = iteration;
				tree.nodes[parentId].lastMergedIn = iteration;
				MergeType mergeType;
				tree.mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);
			}
		}
	}

	TreeType &tree;
	TopTree<DataType> &topTree;
};
