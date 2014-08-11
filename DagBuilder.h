#pragma once

#include <unordered_map>

#include "Common.h"
#include "BinaryDag.h"
#include "TopTree.h"

/// DagNode equality tester, to enable its use in a map
template <typename DataType>
struct SubtreeEquality {
	bool operator()(const DagNode<DataType> &node, const DagNode<DataType> &other) const {
		if ((node.label == NULL) != (other.label == NULL)) {
			return false;
		}
		if ((node.label != NULL) && *(node.label) != *(other.label)) {
			return false;
		}
		return (node.left == other.left) && (node.right == other.right) && (node.mergeType == other.mergeType);
	}
};

/// DagNode hasher to enable its use in a map
template <typename DataType>
struct SubtreeHasher {
	uint operator()(const DagNode<DataType> &node) const {
		uint seed(0);
		if (node.label != NULL)
			boost_hash_combine(seed, *node.label);
		else
			boost_hash_combine(seed, NULL);
		boost_hash_combine(seed, node.left);
		boost_hash_combine(seed, node.right);
		boost_hash_combine(seed, (int)node.mergeType);
		return seed;
	}
};

/// Constructs a top tree's minimal DAG (which is binary)
template <typename DataType>
class DagBuilder {
public:
	DagBuilder(TopTree<DataType> &t, BinaryDag<DataType> &dag)
		: topTree(t), dag(dag), nodeMap(), clusterToDag(t.clusters.size(), -1) {}

	void createDag() {
		// need to add the root separately, it isn't referenced by an edge in the top tree
		dag.nodes[addClusterToDag(0)].label = topTree.clusters[0].label;
		topTree.inPostOrder([&](const int clusterId) { addClusterToDag(clusterId); });
	}

private:
	// add a cluster to the DAG (or just link to it if it's already there)
	int addClusterToDag(const int clusterId) {
		Cluster<DataType> &cluster = topTree.clusters[clusterId];
		assert((cluster.left < 0) == (cluster.right < 0));

		int left(-1), right(-1);
		if (cluster.left != -1) {
			// cluster is NOT a leaf, resolve child nodes in DAG
			left = clusterToDag[cluster.left];
			right = clusterToDag[cluster.right];
			assert(left > 0 && right > 0);
		}
		// Adding the node to the DAG and potentially removing it immediately a few
		// lines down is very cheap, as it's added with emplace_back (object is
		// only constructed once) and the next node would've caused an extension of
		// the vector anyway. Plus, we would've needed the object for checking the
		// hashmap anyway, so this way, we can avoid creating it twice.
		int id = dag.addNode(left, right, cluster.label, cluster.mergeType);
		DagNode<DataType> &node = dag.nodes[id];
		assert(node.left == left && node.right == right && node.label == cluster.label &&
			   node.mergeType == cluster.mergeType);
		int &nodeId = nodeMap[node];  // note the reference which allows avoiding re-hashing for insertion
		if (nodeId == 0) {
			// node is not yet in the hashmap (element 0 is the dummy)
			nodeId = id;
			clusterToDag[clusterId] = id;
			// Increase the childrens' in-degree
			if (left >= 0) dag.nodes[left].inDegree++;
			if (right >= 0) dag.nodes[right].inDegree++;
			return id;
		} else {
			// node is already in the hashmap, compression is happening!
			clusterToDag[clusterId] = nodeId;
			dag.popNode();
			return nodeId;
		}
	}

	TopTree<DataType> &topTree;
	BinaryDag<DataType> &dag;
	std::unordered_map<DagNode<DataType>, int, SubtreeHasher<DataType>, SubtreeEquality<DataType>> nodeMap;
	vector<int> clusterToDag;
};

/// Unpack a binary DAG to its original top tree
template <typename DataType>
class BinaryDagUnpacker {
public:
	BinaryDagUnpacker(BinaryDag<DataType> &dag, TopTree<DataType> &topTree) : nextLeafId(1), dag(dag), topTree(topTree) {}

	void unpack() {
		assert((int)topTree.clusters.size() == topTree.numLeaves);
		dag.template inPostOrder<int>(
			[&](const int nodeId, const int left, const int right) {
				return addNodeToTopTree(nodeId, left, right);
			});
		// Restore the root label
		topTree.clusters[0].label = dag.nodes[1].label;
	}

private:
	int addNodeToTopTree(const int nodeId, const int left, const int right) {
		const DagNode<DataType> &node = dag.nodes[nodeId];
		assert((node.left < 0) == (node.right < 0));
		if (node.left < 0) {
			// it's a leaf
			assert(nextLeafId < topTree.numLeaves);
			int newId = nextLeafId++;
			topTree.clusters[newId].label = node.label;
			return newId;
		} else {
			// add `left` and `right` as children
			assert(node.label == NULL);
			assert(node.mergeType != NO_MERGE);
			return topTree.addCluster(left, right, node.mergeType);
		}
	}

	int nextLeafId;
	BinaryDag<DataType> &dag;
	TopTree<DataType> &topTree;
};
