#pragma once

#include "Labels.h"

/// Hash a node for RePair combiner
template <typename TreeType, typename DataType>
struct NodeHasher {
	/// Create hasher for a tree and its tentative Top DAG
	/// \param tree The input tree
	/// \param topDag An empty Top DAG
	/// \param nodeIds An empty mapping from tree nodes to Top DAG clusters
	NodeHasher(TreeType &tree, const TopDag<DataType> &topDag, const std::vector<int> &nodeIds) :
		tree(tree), topDag(topDag), nodeIds(nodeIds), cache(tree._numNodes * 2, 0) {}

	/// Hash a node
	/// \param nodeId node identified by its tree node ID
	void hashNode(const int nodeId) {
		assert(nodeId < tree._numNodes);
		tree.nodes[nodeId].hash = hashCluster(nodeIds[nodeId]);
	}

	/// Hash a cluster
	/// \param clusterId cluster identified by its Top DAG cluster ID
	/// \param returns the hash value (which is also set)
	uint hashCluster(const int clusterId) {
		assert(clusterId < (int)topDag.clusterToDag.size());

		uint hash = 0;
		const int nodeId = topDag.clusterToDag[clusterId];
		const DagNode<DataType> &dagNode = topDag.nodes[nodeId];

		// Hash merge type
		boost_hash_combine(hash, (int)dagNode.mergeType);

		// Hash label
		if (dagNode.label != NULL) {
			assert(dagNode.left < 0 && dagNode.right < 0);
			boost_hash_combine<DataType>(hash, *dagNode.label);
		} else {
			assert(dagNode.left >= 0 && dagNode.right >= 0);
			assert(cache[dagNode.left] > 0 && cache[dagNode.right] > 0);
			boost_hash_combine(hash, cache[dagNode.left]);
			boost_hash_combine(hash, cache[dagNode.right]);
		}

		cache[nodeId] = hash;
		return hash;
	}

	/// Hash the entire tree in post-order
	void hashTree(const int nodeId = 0) {
		for (auto *edge = tree.firstEdge(nodeId); edge <= tree.lastEdge(nodeId); ++edge) {
			if (edge->valid) {
				hashTree(edge->headNode);
			}
		}
		hashNode(nodeId);
	}

	TreeType &tree;
	const TopDag<DataType> &topDag;
	const std::vector<int> &nodeIds;
	std::vector<uint> cache;
};
