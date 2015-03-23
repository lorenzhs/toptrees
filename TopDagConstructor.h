#pragma once

#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>

#include "Timer.h"
#include "TopDag.h"
#include "Statistics.h"

using std::cout;
using std::endl;
using std::flush;
using std::vector;

/// Transform a tree into its top tree
/**
 * Given a tree (currently, only OrderedTree is supported), construct
 * its top tree iteratively.
 *
 * The original tree will be modified heavily in the process!
 * When transformation is complete, only one edge will remain
 * and nodes' parent values will be lost as well.
 * In short, this destroys the input tree.
 */
template <typename TreeType, typename DataType>
class TopDagConstructor {
	typedef typename TreeType::nodeType NodeType;
	typedef typename TreeType::edgeType EdgeType;

public:
	/// Instantiate a top tree constructor
	/// \param tree the tree which shall be transformed. WILL BE MODIFIED
	/// \param topDag the output top tree
	/// \param verbose whether to print detailed information about the iterations
	/// \param extraVerbose whether to print the tree in each iteration
	TopDagConstructor(TreeType &tree, TopDag<DataType> &topDag, const bool verbose = true, const bool extraVerbose = false)
		: tree(tree), topDag(topDag), verbose(verbose), extraVerbose(extraVerbose), nodeIds(tree._numNodes) {}

	/// Perform the top tree construction procedure
	/// \param debugInfo pointer to a DebugInfo object, should you wish logging of debug information
	void construct(DebugInfo *debugInfo = NULL) {
		for (int i = 0; i < tree._numNodes; ++i) {
			nodeIds[i] = i;
		}

		doMerges(debugInfo);
	}

protected:
	void mergeCallback(const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topDag.addCluster(nodeIds[u], nodeIds[v], type);
	}

	/// do iterated merges to construct a top tree
	/// \param mergeCallback the callback that will be called for every pair of merged nodes (clusters).
	/// Its arguments are the ids of the two merged nodes and the new id
	/// (usually one of the two old ones) as well as the type of the merge
	/// \param debugInfo the DebugInfo object or NULL
	/// \param verbose whether to print detailed information about the iterations
	/// \param extraVerbose whether to print the tree in each iteration
	void doMerges(DebugInfo *debugInfo) {
		int iteration = 0;
		Timer timer;
		const std::streamsize precision = cout.precision();
		cout << std::fixed << std::setprecision(1);
		while (tree._numEdges > 1) {
			if (verbose) cout << "It. " << std::setw(2) << iteration << ": merging horz… " << flush;

			if (extraVerbose) cout << endl << tree.shortString() << endl;

			int oldNumEdges = tree._numEdges;
#ifdef SWEEP
			horizontalMergesAllPairs(iteration);
#else
			horizontalMerges(iteration);
#endif
			tree.killNodes();
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; vert… " << flush;

			verticalMerges(iteration);
			tree.killNodes();
			if (verbose) cout << std::setw(6) << timer.getAndReset() << " ms; " << tree.summary();

			double ratio = (oldNumEdges * 1.0) / tree._numEdges;
			if (verbose && ratio < 1.2) cout << " ratio " << std::setprecision(5) << ratio << std::setprecision(1) << std::endl << tree.shortString();
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

	/// Do one iteration of horizontal merges (step 1)
	void horizontalMerges(const int iteration) {
		for (int nodeId = tree._numNodes - 1; nodeId >= 0; --nodeId) {
			// merging children only make sense for nodes with ≥ 2 children
			const int numEdges(tree.nodes[nodeId].numEdges());
			if (numEdges < 2) {
				continue;
			}
#ifndef NDEBUG
			// verify that these edges indeed do belong to whoever claims to be their parent
			for (EdgeType *edge = tree.firstEdge(nodeId); edge < tree.lastEdge(nodeId); ++edge) {
				assert(tree.nodes[edge->headNode].parent == nodeId);
			}
#endif
			bool hasMerged=false;
			EdgeType *leftEdge, *rightEdge, *baseEdge(tree.firstEdge(nodeId));
			int left, right, newNode, edgeNum;
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
					assert(tree.nodes[left].lastMergedIn < iteration);
					assert(tree.nodes[right].lastMergedIn < iteration);
					tree.nodes[left].lastMergedIn = iteration;
					tree.nodes[right].lastMergedIn = iteration;
					tree.mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
					hasMerged = true;
				}
			}

			if (edgeNum == numEdges - 1) {
				// the node has an odd number of children. check if the conditions for an odd
				// merge at the end are satisfied. We need the last child to be a leaf and the
				// two previous children to be non-leaves. This implies that they have not been
				// merged in this iteration so far (because neither is a leaf)
				leftEdge = tree.lastEdge(nodeId);
				left = leftEdge->headNode;
				if (tree.nodes[left].isLeaf() && tree.nodes[nodeId].numEdges() > 2 && (leftEdge - 1)->valid && (leftEdge - 2)->valid) {
					const int childMinusOne = (leftEdge - 1)->headNode;
					const int childMinusTwo = (leftEdge - 2)->headNode;
					if (!tree.nodes[childMinusOne].isLeaf() && !tree.nodes[childMinusTwo].isLeaf()) {
						// Everything is go for a merge in the "odd case"
						assert(tree.nodes[left].lastMergedIn < iteration);
						assert(tree.nodes[childMinusOne].lastMergedIn < iteration);
						tree.nodes[left].lastMergedIn = iteration;
						tree.nodes[childMinusOne].lastMergedIn = iteration;
						tree.mergeSiblings(leftEdge - 1, leftEdge, newNode, mergeType);
						mergeCallback(childMinusOne, left, newNode, mergeType);
						hasMerged = true;
					}
				}
			}
			if (hasMerged) {
				tree.compactNode(nodeId);
			}
		}
	}

	/// Do one iteration of horizontal merges (step 1)
	/// Modified to look at (1,2), (2,3), (3,4) etc instead of (1,2), (3,4), etc
	void horizontalMergesAllPairs(const int iteration) {
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
			EdgeType *baseEdge(tree.firstEdge(nodeId));
			int newNode, lastEdgeNum(node.numEdges() - 1);
			MergeType mergeType;
			// iterate over pairs of children by index
			for (int edgeNum = 0; edgeNum < lastEdgeNum; ++edgeNum) {
				EdgeType *leftEdge(baseEdge + edgeNum);
				EdgeType *rightEdge(leftEdge + 1);
				assert(leftEdge->valid && rightEdge->valid);
				const int left = leftEdge->headNode;
				const int right = rightEdge->headNode;
				// We can only merge if at least one of the two is a leaf
				if (tree.nodes[left].isLeaf() || tree.nodes[right].isLeaf()) {
					assert(tree.nodes[left].lastMergedIn < iteration);
					assert(tree.nodes[right].lastMergedIn < iteration);
					tree.nodes[left].lastMergedIn = iteration;
					tree.nodes[right].lastMergedIn = iteration;
					tree.mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
					++edgeNum;
				}
			}
		}
	}

	/// Perform an iteration of vertical (chain) merges (step 2)
	void verticalMerges(const int iteration) {
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

			while (parentId >= 0 && tree.nodes[parentId].hasOnlyOneChild()) {
				NodeType &node(tree.nodes[nodeId]), &parent(tree.nodes[parentId]);

				if (node.lastMergedIn == iteration || parent.lastMergedIn == iteration) {
					nodeId = parentId;
					parentId = parent.parent;
					continue;
				}

				assert(node.lastMergedIn < iteration);
				assert(parent.lastMergedIn < iteration);
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
		}
	}

	TreeType &tree;
	TopDag<DataType> &topDag;
	const bool verbose, extraVerbose;
	vector<int> nodeIds;
};
