#pragma once

#include <cassert>
#include <ostream>
#include <functional>
#include <vector>

#include "Common.h"

using std::function;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

struct Cluster {
	Cluster(): mergeType(NO_MERGE), left(-1), right(-1), label(NULL) /*, rLeft(-1) , lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	Cluster(int l, int r, MergeType t): mergeType(t), left(l), right(r), label(NULL) /*, rLeft(-1), lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	MergeType mergeType;
	int left, right /*, rLeft, lRight, rRight, height, size, distTBleft, distTBright*/;
	string* label;

	friend ostream& operator<<(ostream& os, const Cluster &cluster) {
		return os << "(" << cluster.left << "," << cluster.right << "/" << cluster.mergeType << "; " << (cluster.label == NULL ? "NULL" : *cluster.label) << ")";
	}
};

struct TopTree {
	TopTree(const int numLeaves, vector<string> &labels): clusters(numLeaves), labels(labels), numLeaves(numLeaves) {
		for (int i = 0; i < numLeaves; ++i) {
			clusters[i].label = &labels[i];
		}
	}

	int addCluster(const int left, const int right, const MergeType mergeType) {
		clusters.push_back(Cluster(left, right, mergeType));
		return clusters.size() - 1;
	}

	void inPostOrder(const function<void (const int)> &callback) {
		traverseTreePostOrder(clusters.size() - 1, callback);
	}

	void traverseTreePostOrder(const int clusterId, const function<void (const int)> &callback) {
		Cluster &cluster = clusters[clusterId];
		if (cluster.left != -1) {
			traverseTreePostOrder(cluster.left, callback);
		}
		if (cluster.right != -1) {
			traverseTreePostOrder(cluster.right, callback);
		}
		callback(clusterId);
	}

	friend ostream& operator<<(ostream& os, const TopTree &toptree) {
		os << "Top tree with " << toptree.clusters.size() << " clusters. Non-leaves:" << endl;
		uint count = 0;
		for (const Cluster &cluster : toptree.clusters) {
			if (cluster.left >= 0) {
				os << "\t" << count << ": " << cluster << endl;
			}
			count++;
		}
		return os;
	}

	vector<Cluster> clusters;
	vector<string> &labels;
	int numLeaves;
};

template<typename TreeType>
class TopTreeUnpacker {
public:
	TopTreeUnpacker(TopTree &topTree, TreeType &tree): topTree(topTree), tree(tree) {
		assert(tree._numNodes == 0);
	}

	void unpack() {
		int firstId = tree.addNodes(topTree.numLeaves);
		assert (firstId == 0);
		int rootCluster = topTree.clusters.size() - 1;
		unpackCluster(rootCluster, 0);
	}

private:
	bool isLeaf(const int clusterId) const {
		return clusterId < topTree.numLeaves;
	}

	int unpackCluster(const int clusterId, const int nodeId) {
		const Cluster& cluster = topTree.clusters[clusterId];
		int leafId;
		switch (cluster.mergeType) {
			case VERT_NO_BBN:
			case VERT_WITH_BBN: leafId = unpackVertCluster(clusterId, nodeId); break;
			case HORZ_NO_BBN:
			case HORZ_LEFT_BBN:
			case HORZ_RIGHT_BBN: leafId = unpackHorzCluster(clusterId, nodeId); break;
			default: assert(false);
		}
		return leafId;
	}

	int unpackVertCluster(const int clusterId, const int nodeId) {
		const Cluster &cluster = topTree.clusters[clusterId];
		int boundaryNode(nodeId);
		if (isLeaf(cluster.left)) {
			tree.addEdge(nodeId, cluster.left);
			boundaryNode = cluster.left;
		} else {
			boundaryNode = unpackCluster(cluster.left, nodeId);
		}

		if (isLeaf(cluster.right)) {
			tree.addEdge(cluster.left, cluster.right);
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
		const Cluster &cluster = topTree.clusters[clusterId];
		int left, right;
		if (isLeaf(cluster.left)) {
			tree.addEdge(nodeId, cluster.left);
			left = cluster.left;
		} else {
			left = unpackCluster(cluster.left, nodeId);
		}

		if (isLeaf(cluster.right)) {
			tree.addEdge(nodeId, cluster.right);
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

	TopTree &topTree;
	TreeType &tree;
};
