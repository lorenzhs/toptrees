#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>

#include "BinaryDag.h"
#include "TopTree.h"

using std::cout;
using std::endl;
using std::unordered_map;

template<typename DataType>
struct SubtreeEquality {
	bool operator() (const DagNode<DataType> &node, const DagNode<DataType> &other) const {
		if ((node.label != NULL) && *(node.label) != *(other.label)) {
			return false;
		}
		return (node.left == other.left) && (node.right == other.right);
	}
};

template<typename DataType>
struct SubtreeHasher {
	template<typename T>
	void boost_hash_combine(uint& seed, const T& val) const {
		std::hash<T> hasher;
		seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	uint operator() (const DagNode<DataType> &node) const {
		uint seed(0);
		if (node.label != NULL)
			boost_hash_combine(seed, *node.label);
		else
			boost_hash_combine(seed, NULL);
		boost_hash_combine(seed, node.left);
		boost_hash_combine(seed, node.right);
		return seed;
	}
};

template<typename DataType>
class DagBuilder {
public:
	DagBuilder(TopTree &t, BinaryDag<DataType> &dag): topTree(t), dag(dag), nodeMap(), clusterToDag(t.clusters.size(), -1) {}

	void createDag() {
		topTree.inPostOrder([&] (const int clusterId){
			addClusterToDag(clusterId);
		});
	}

	int addClusterToDag(const int clusterId) {
		Cluster &cluster = topTree.clusters[clusterId];
		assert ((cluster.left < 0) == (cluster.right < 0));

		int left(-1), right(-1);
		if (cluster.left != -1) {
			// cluster is NOT a leaf, resolve child nodes in DAG
			left = clusterToDag[cluster.left];
			right = clusterToDag[cluster.right];
			assert(left >= 0 && right >= 0);
		}
		// Adding the node to the DAG and potentially removing it immediately a few
		// lines down is very cheap, as it's added with emplace_back (object is
		// only constructed once) and the next node would've caused an extension of
		// the vector anyway. Plus, we would've needed the object for checking the
		// hashmap anyway, so this way, we can avoid creating it twice.
		int id = dag.addNode(left, right, cluster.label);
		DagNode<DataType> &node = dag.nodes[id];
		int nodeId = nodeMap[node];
		if (nodeId == 0) {
			// node is not yet in the hashmap (element 0 is the dummy)
			nodeMap[node] = id;
			clusterToDag[clusterId] = id;
			return id;
		} else {
			// node is already in the hashmap, compression is happening!
			// we should probably increase the indegree instead of the childrens
			// indegrees? so move the child indegree increase to the if case and the
			// node indegree increase here? TODO figure this out tomorrow, it's late
			clusterToDag[clusterId] = nodeId;
			dag.popNode();
			return nodeId;
		}
	}

	TopTree &topTree;
	BinaryDag<DataType> &dag;
	unordered_map<DagNode<DataType>, int, SubtreeHasher<DataType>, SubtreeEquality<DataType>> nodeMap;
	vector<int> clusterToDag;
};
