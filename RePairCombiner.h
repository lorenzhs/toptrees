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

#include "RePair.h"
#include "RePairTreeHasher.h"

using std::cout;
using std::endl;
using std::flush;
using std::function;
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
class RePairCombiner {
	typedef typename TreeType::nodeType NodeType;
	typedef typename TreeType::edgeType EdgeType;

	struct Pair {
		Pair(int parentId, int leftEdgeIndex) : parentId(parentId), leftEdgeIndex(leftEdgeIndex) {}
		int parentId, leftEdgeIndex;
	};

public:
	/// Instantiate a top tree constructor
	/// \param tree the tree which shall be transformed. WILL BE MODIFIED
	/// \param topTree the output top tree
	/// \param verbose whether to print detailed information about the iterations
	/// \param extraVerbose whether to print the tree in each iteration
	RePairCombiner(TreeType &tree, TopTree<DataType> &topTree, const LabelsT<DataType> &labels, const bool verbose = true, const bool extraVerbose = false)
		: tree(tree), topTree(topTree), verbose(verbose), extraVerbose(extraVerbose), nodeIds(tree._numNodes), hasher(tree, labels) {}

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
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
		hasher.hashNode(n);
	}


	/// do iterated merges to construct a top tree
	/// \param mergeCallback the callback that will be called for every pair of merged nodes (clusters).
	/// Its arguments are the ids of the two merged nodes and the new id
	/// (usually one of the two old ones) as well as the type of the merge
	/// \param debugInfo the DebugInfo object or NULL
	/// \param verbose whether to print detailed information about the iterations
	/// \param extraVerbose whether to print the tree in each iteration
	void doMerges(DebugInfo *debugInfo) {
		hasher.hash();

		int iteration = 0;
		Timer timer;

		const std::streamsize precision = cout.precision();
		cout << std::fixed << std::setprecision(1);
		while (tree._numEdges > 1) {
			if (verbose) cout << "It. " << std::setw(2) << iteration << ": merging horz… " << flush;

			if (extraVerbose) cout << endl << tree.shortString() << endl;

			int oldNumEdges = tree._numEdges;
			// First, do RePair merges, then whatever else is possible
			horizontalMergesRePair();
			normalHorizontalMerges();
			tree.killNodes();
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; gc… " << flush;

			// We need to compact here because the horizontal merges don't but
			// the vertical merges need correct edge counts, so this is important!
			// And it turns out that rebuild compation is faster than inplace,
			// maybe because of subsequent cache efficiency?
			tree.compact(false);
			if (verbose) cout << std::setw(6) << timer.getAndReset() << "ms; vert… " << flush;

			verticalMerges();
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


	uint getRePairHash(const EdgeType *edge) const {
		const uint leftHash(tree.nodes[ edge   ->headNode].hash),
				  rightHash(tree.nodes[(edge+1)->headNode].hash);
		return RePair::HashCombiner::hash(leftHash, rightHash);
	}

	void prepareRePair(RePair::HashMap<Pair> &hashMap, RePair::PriorityQueue<Pair> &queue) {
		// Populate the HashMap with the pairs
		uint numPairs(0);
		for (int nodeId = 0; nodeId < tree._numNodes; ++nodeId) {
			for (EdgeType *edge = tree.firstEdge(nodeId); edge < tree.lastEdge(nodeId); ++edge) {
				assert((edge+1)->valid);
				if (!tree.nodes[edge->headNode].isLeaf() && !tree.nodes[(edge+1)->headNode].isLeaf()) {
					// We're only interested in merging if one is a leaf
					continue;
				}
				Pair pair(nodeId, tree.edgeId(edge));
				hashMap.add(getRePairHash(edge), pair);
				++numPairs;
			}
		}

		const int queueSize(sqrt(numPairs));
		queue.init(queueSize);
		hashMap.populatePQ(queue);
	}

	void horizontalMergesRePair() {
		RePair::Records<Pair> records;
		RePair::HashMap<Pair> hashMap(records);
		RePair::PriorityQueue<Pair> queue;
		prepareRePair(hashMap, queue);

		while (!queue.empty()) {
			RePair::Record<Pair> *record = queue.popMostFrequentRecord();
			//cout << "Processing record " << *record << endl;
			for (Pair pair : record->occurrences) {
				//cout << "\tProcessing pair (" << pair.leftEdgeIndex << ", " << pair.parentId << ")" << endl;
				const int leftEdge = pair.leftEdgeIndex;
				const int rightEdge = leftEdge + 1;
				if (!tree.edges[leftEdge].valid || !tree.edges[rightEdge].valid) {
					// looks like we hit an overlapping pair, meh
					continue;
				}


				// Decrement frequencies of neighbouring pairs
				if (leftEdge > tree.nodes[pair.parentId].firstEdgeIndex) {
					if (tree.edges[leftEdge - 1].valid) {
						const uint hash = getRePairHash(&tree.edges[leftEdge - 1]);
						auto *record = &hashMap.records.records[hashMap.recordMap[hash]];
						queue.decrementFrequency(record);
					}
				}
				if (rightEdge < tree.nodes[pair.parentId].lastEdgeIndex) {
					if (tree.edges[rightEdge + 1].valid) {
						const uint hash = getRePairHash(&tree.edges[rightEdge]);
						auto *record = &hashMap.records.records[hashMap.recordMap[hash]];
						queue.decrementFrequency(record);
					}
				}

				// Do the merge
				MergeType mergeType;
				int newNode;
				tree.mergeSiblings(&tree.edges[leftEdge], &tree.edges[rightEdge], newNode, mergeType);
				mergeCallback(tree.edges[leftEdge].headNode, tree.edges[rightEdge].headNode, newNode, mergeType);
			}
		}
	}

	void normalHorizontalMerges() {
		// Do the rest of the horizontal merges
		for (int nodeId = tree._numNodes - 1; nodeId >= 0; --nodeId) {
			const int numEdges(tree.nodes[nodeId].numEdges());
			// merging children only make sense for nodes with ≥ 2 children
			if (numEdges < 2) {
				continue;
			}
#ifndef NDEBUG
			// verify that these edges indeed do belong to whoever claims to be their parent
			for (EdgeType *edge = tree.firstEdge(nodeId); edge < tree.lastEdge(nodeId); ++edge)
				if (edge->valid)
					assert(tree.nodes[edge->headNode].parent == nodeId);
#endif
			NodeType &node(tree.nodes[nodeId]);
			EdgeType *baseEdge(tree.firstEdge(nodeId));
			int newNode, lastEdgeNum(node.numEdges() - 1);
			MergeType mergeType;
			// iterate over pairs of children by index
			for (int edgeNum = 0; edgeNum < lastEdgeNum; ++edgeNum) {
				EdgeType *leftEdge(baseEdge + edgeNum);
				if (!leftEdge->valid) {
					continue;
				}
				EdgeType *rightEdge(leftEdge + 1);
				if (!rightEdge->valid) {
					++edgeNum;
					continue;
				}
				const int left = leftEdge->headNode;
				const int right = rightEdge->headNode;
				// We can only merge if at least one of the two is a leaf
				if ((tree.nodes[left].isLeaf() || tree.nodes[right].isLeaf())) {
					tree.mergeSiblings(leftEdge, rightEdge, newNode, mergeType);
					mergeCallback(left, right, newNode, mergeType);
					++edgeNum;
				}
			}
		}
	}

	/// Perform an iteration of vertical (chain) merges (step 2)
	void verticalMerges() {
		// First, we collect all the vertices from which a merge chain can originate upwards
		// This is needed to prevent repeated merges of the same chain in one iteration
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
				   tree.nodes[tree.nodes[parentId].parent].hasOnlyOneChild()) {
				MergeType mergeType;
				tree.mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);

				// Follow the chain upwards if possible
				nodeId = tree.nodes[parentId].parent;
				if (nodeId >= 0) {
					parentId = tree.nodes[nodeId].parent;
				} else {
					break; // break while loop
				}
			}

			if (nodeId >= 0 && parentId >= 0 && tree.nodes[parentId].hasOnlyOneChild() &&
				tree.nodes[parentId].parent >= 0) {
				// We hit the "odd case"
				assert(!tree.nodes[tree.nodes[parentId].parent].hasOnlyOneChild());
				MergeType mergeType;
				tree.mergeChain(parentId, mergeType);
				mergeCallback(parentId, nodeId, parentId, mergeType);
			}
		}
	}

	TreeType &tree;
	TopTree<DataType> &topTree;
	const bool verbose, extraVerbose;
	vector<int> nodeIds;
	NodeHasher<TreeType, DataType> hasher;
};
