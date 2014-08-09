#pragma once

#include <cassert>

#include "Labels.h"
#include "TopTree.h"

/// Unpack a TopTree into its original OrderedTree
template <typename TreeType, typename DataType>
class TopTreeUnpacker {
public:
	TopTreeUnpacker(TopTree<DataType> &topTree, TreeType &tree, LabelsT<DataType> &labels)
		: topTree(topTree), tree(tree), labels(labels) {
		assert(tree._numNodes == 0);
	}

	void unpack() {
		int firstId = tree.addNodes(topTree.numLeaves);
		assert(firstId == 0);

		// special treatment for root node of original tree
		const DataType *label = topTree.clusters[0].label;
		assert(label != NULL);
		labels.set(0, *label);
		// unpack the rest
		unpackCluster(topTree.clusters.size() - 1, firstId);
	}

private:
	bool isLeaf(const int clusterId) const {
		return clusterId < topTree.numLeaves;
	}

	void handleLeaf(const int leafId) {
		const DataType *label = topTree.clusters[leafId].label;
		assert(label != NULL);
		labels.set(leafId, *label);
	}

	int unpackCluster(const int clusterId, const int nodeId) {
		const Cluster<DataType> &cluster = topTree.clusters[clusterId];
		int leafId;
		switch (cluster.mergeType) {
		case VERT_NO_BBN:
		case VERT_WITH_BBN:
			leafId = unpackVertCluster(clusterId, nodeId);
			break;
		case HORZ_NO_BBN:
		case HORZ_LEFT_BBN:
		case HORZ_RIGHT_BBN:
			leafId = unpackHorzCluster(clusterId, nodeId);
			break;
		default:
			assert(false);
			leafId = -1; // to make the compiler happy with NDEBUG
		}

		// Perform compaction because the OrderedTree data structure is really unsuitable
		// for decompression of the tree.
		if (tree._firstFreeEdge - tree._numEdges > 100000000) {
			tree.compact(true, 3);
		}

		return leafId;
	}

	int unpackVertCluster(const int clusterId, const int nodeId) {
		const Cluster<DataType> &cluster = topTree.clusters[clusterId];
		int boundaryNode(nodeId);
		if (isLeaf(cluster.left)) {
			tree.addEdge(nodeId, cluster.left, extraSpace);
			handleLeaf(cluster.left);
			boundaryNode = cluster.left;
		} else {
			boundaryNode = unpackCluster(cluster.left, nodeId);
		}

		if (isLeaf(cluster.right)) {
			tree.addEdge(boundaryNode, cluster.right, extraSpace);
			handleLeaf(cluster.right);
			boundaryNode = cluster.right;
		} else {
			boundaryNode = unpackCluster(cluster.right, boundaryNode);
		}

		if (cluster.mergeType == VERT_WITH_BBN)
			return boundaryNode;
		else
			return -1;
	}

	int unpackHorzCluster(const int clusterId, const int nodeId) {
		const Cluster<DataType> &cluster = topTree.clusters[clusterId];
		int left, right;
		if (isLeaf(cluster.left)) {
			tree.addEdge(nodeId, cluster.left, extraSpace);
			handleLeaf(cluster.left);
			left = cluster.left;
		} else {
			left = unpackCluster(cluster.left, nodeId);
		}

		if (isLeaf(cluster.right)) {
			tree.addEdge(nodeId, cluster.right, extraSpace);
			handleLeaf(cluster.right);
			right = cluster.right;
		} else {
			right = unpackCluster(cluster.right, nodeId);
		}

		if (cluster.mergeType == HORZ_LEFT_BBN)
			return left;
		else if (cluster.mergeType == HORZ_RIGHT_BBN)
			return right;
		else
			return -1;
	}

	TopTree<DataType> &topTree;
	TreeType &tree;
	LabelsT<DataType> &labels;
	const int extraSpace = 50;
};
