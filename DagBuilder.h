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
	// adapted from: http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
	// the magic number is the binary extension of the golden ratio
	// the idea is that every bit is random. A more detailed explanation is available at
	// http://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
	template<typename T>
	static void boost_hash_combine(uint& seed, const T& val) {
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
			// Increase the childrens' in-degree
			if (left >= 0)
				dag.nodes[left].inDegree++;
			if (right >= 0)
				dag.nodes[right].inDegree++;
			return id;
		} else {
			// node is already in the hashmap, compression is happening!
			clusterToDag[clusterId] = nodeId;
			dag.nodes[nodeId].inDegree++;
			dag.popNode();
			return nodeId;
		}
	}

	TopTree &topTree;
	BinaryDag<DataType> &dag;
	unordered_map<DagNode<DataType>, int, SubtreeHasher<DataType>, SubtreeEquality<DataType>> nodeMap;
	vector<int> clusterToDag;
};


template<typename DataType>
class BinaryDagUnpacker {
public:
	BinaryDagUnpacker(BinaryDag<DataType> &dag, TopTree &topTree): nextLeafId(0), dag(dag), topTree(topTree) {}

	void unpack() {
		assert((int) topTree.clusters.size() == topTree.numLeaves);
		dag.template inPostOrder<int>([&] (const int nodeId, const int left, const int right) {
			return addNodeToTopTree(nodeId, left, right);
		});
	}

private:
	int addNodeToTopTree(const int nodeId, const int left, const int right) {
		const DagNode<DataType> &node = dag.nodes[nodeId];
		assert((node.left < 0) == (node.right < 0));
		if (node.left < 0) {
			// it's a leaf
			int newId = nextLeafId++;
			topTree.clusters[newId].label = node.label;
			return newId;
		} else {
			// add `left` and `right` as children
			// XXX TODO how do we get the merge type back?
			assert(node.label == NULL);
			return topTree.addCluster(left, right, NO_MERGE);
		}
		return -1;
	}

	int nextLeafId;
	BinaryDag<DataType> &dag;
	TopTree &topTree;
};
