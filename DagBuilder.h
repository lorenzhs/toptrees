#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>

#include "BinaryDag.h"
#include "TopTree.h"

using std::cout;
using std::endl;
using std::unordered_map;

/*
template<typename DataType>
struct SubtreeEquality {
	bool operator() (const DagNode<DataType> *node, const DagNode<DataType> *other) const {
		//cout << "labels " << *node << " and " << *other << " equal: " << (node->label == other->label) << " left: " << (node->left == other->left) << " right: " << (node->right == other->right) << endl;
		if (node == NULL) {
			cout << "NODE IS NULL" << endl;
		}
		if (other == NULL) {
			cout << "OTHER IS NULL" << endl;
		}
		cout << "Comparing nodes " << *node << " and " << *other << endl;
		assert(node != NULL && other != NULL);
		if ((node->label == NULL) != (other->label == NULL)) {
			cout << "one is null, the other isn't" << endl;
			return false;
		}
		if ((node->label != NULL) && *(node->label) != *(other->label)){
			cout << "both not null: " << *node->label << " and " << *other->label << endl;
			return false;
		}
		return (node->left == other->left) && (node->right == other->right);
	}
};
*/

template<typename DataType>
struct SubtreeEquality {
	bool operator() (const DagNode<DataType> &node, const DagNode<DataType> &other) const {
		//cout << "labels " << *node << " and " << *other << " equal: " << (node->label == other->label) << " left: " << (node->left == other->left) << " right: " << (node->right == other->right) << endl;
		if ((node.label != NULL) && *(node.label) != *(other.label)){
			//cout << "both not null: " << *node.label << " and " << *other.label << endl;
			return false;
		}
		return (node.left == other.left) && (node.right == other.right);
	}
};

/*
template<typename DataType>
struct SubtreeHasher {
	// adapted from: http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
	// the magic number is the binary extension of the golden ratio
	// the idea is that every bit is random. A more detailed explanation is available at
	// http://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
	template<typename T>
	void boost_hash_combine(uint& seed, const T& val) const {
		std::hash<T> hasher;
		seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	uint operator() (const DagNode<DataType> *node) const {
		uint seed(0);
		if (node == NULL) return 0;
		if (node->label != NULL)
			boost_hash_combine(seed, *node->label);
		else
			boost_hash_combine(seed, NULL);
		boost_hash_combine(seed, node->left);
		boost_hash_combine(seed, node->right);
		cout << "hash of " << *node << ": " << seed << endl;
		return seed;
	}
};
*/

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
		//cout << "hash of " << node << ": " << seed << endl;
		return seed;
	}
};

template<typename DataType>
class DagBuilder {
public:
	DagBuilder(TopTree &t, BinaryDag<DataType> &dag): topTree(t), dag(dag), nodeMap(), clusterToDag(t.clusters.size(), -1) {}

	void createDag() {
		topTree.inPostOrder([&] (const int clusterId){
			//cout << "processing cluster " << clusterId << ": ";
			addClusterToDag(clusterId);
		});
	}

	int addClusterToDag(const int clusterId) {
		Cluster &cluster = topTree.clusters[clusterId];
		assert ((cluster.left < 0) == (cluster.right < 0));

		int left(-1), right(-1);
		if (cluster.left != -1) {
			// cluster is NOT a leaf
			left = clusterToDag[cluster.left];
			right = clusterToDag[cluster.right];
			assert(left >= 0 && right >= 0);
		}
		int id = dag.addNode(left, right, cluster.label);
		DagNode<DataType> &node = dag.nodes[id];
		int nodeId = nodeMap[node];
		//cout << " cluster: " << cluster << " left: " << left << " right: " << right << " node: " << node << std::flush;
		//int id = nodeMap[&node];
		//cout << " map returned " << nodeMap[node];
		if (nodeId == 0) {
			// TODO don't create object twice
			nodeMap[node] = id;
			clusterToDag[clusterId] = id;
			//cout << " added into map as " << id << endl;
			return id;
		} else {
			//cout << " in map with id " << nodeId << endl;
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

