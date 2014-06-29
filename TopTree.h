#pragma once

#include "Common.h"

struct Cluster {
	Cluster(): mergeType(NO_MERGE), left(-1), right(-1) /*, rLeft(-1) , lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	Cluster(int l, int r, MergeType t): mergeType(t), left(l), right(r) /*, rLeft(-1), lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	MergeType mergeType;
	int left, right /*, rLeft, lRight, rRight, height, size, distTBleft, distTBright*/;

	friend std::ostream& operator<<(std::ostream& os, const Cluster &cluster) {
		return os << "(" << cluster.left << "," << cluster.right << "/" << cluster.mergeType << ")";
	}
};

struct TopTree {
	TopTree(const int numLeaves): clusters(numLeaves) {}

	int addCluster(const int left, const int right, const MergeType mergeType) {
		clusters.push_back(Cluster(left, right, mergeType));
		return clusters.size() - 1;
	}

	friend std::ostream& operator<<(std::ostream& os, const TopTree &toptree) {
		os << "Top tree with " << toptree.clusters.size() << " clusters. Non-leaves:" << endl;
		uint count = 0;
		for (const Cluster &cluster: toptree.clusters) {
			if (cluster.left >= 0) {
				os << "\t" << count << ": " << cluster << endl;
			}
			count++;
		}
		return os;
	}	

	vector<Cluster> clusters;
};
