#pragma once

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
	TopTree(const int numLeaves, vector<string> &labels): clusters(numLeaves), labels(labels) {
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
};
