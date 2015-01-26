#pragma once

#include "Labels.h"

template <typename TreeType, typename DataType>
struct NodeHasher {
	NodeHasher(TreeType &tree, const TopTree<DataType> &topTree, const std::vector<int> &nodeIds) :
		tree(tree), topTree(topTree), nodeIds(nodeIds), cache(tree._numNodes * 2, 0) {}

	void prepare() {
		for (int i = 0; i < tree._numNodes; ++i) {
			hashNode(i);
		}
	}

	void hashNode(const int nodeId) {
		assert(nodeId < tree._numNodes);
		tree.nodes[nodeId].hash = hashCluster(nodeIds[nodeId]);
	}

	uint hashCluster(const int clusterId) {
		assert(clusterId < (int)topTree.clusters.size());

		uint hash = 0;
		const Cluster<DataType> &cluster = topTree.clusters[clusterId];

		// Hash merge type
		boost_hash_combine(hash, (int)cluster.mergeType);

		// Hash label
		if (cluster.label != NULL) {
			assert(cluster.left < 0 && cluster.right < 0);
			boost_hash_combine<DataType>(hash, *cluster.label);
		} else {
			assert(cluster.left >= 0 && cluster.right >= 0);
			assert(cache[cluster.left] > 0 && cache[cluster.right] > 0);
			boost_hash_combine(hash, cache[cluster.left]);
			boost_hash_combine(hash, cache[cluster.right]);
		}

		cache[clusterId] = hash;
		return hash;
	}

	void hashTree(const int nodeId = 0) {
		for (auto *edge = tree.firstEdge(nodeId); edge <= tree.lastEdge(nodeId); ++edge) {
			if (edge->valid) {
				hashTree(edge->headNode);
			}
		}
		hashNode(nodeId);
	}

	TreeType &tree;
	const TopTree<DataType> &topTree;
	const std::vector<int> &nodeIds;
	std::vector<uint> cache;
};
