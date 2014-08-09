#pragma once

#include <cassert>
#include <iostream>
#include <functional>
#include <vector>

#include "Common.h"
#include "Labels.h"

using std::function;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

/// Cluster type for a top tree, holding a pointer to some data
template <typename DataType>
struct Cluster {
	Cluster() : mergeType(NO_MERGE), left(-1), right(-1), label(NULL)
		/*, rLeft(-1), lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	Cluster(int l, int r, MergeType t) : mergeType(t), left(l), right(r), label(NULL)
		/*, rLeft(-1), lRight(-1), rRight(-1), height(-1), size(-1), distTBleft(-1), distTBright(-1)*/ {}
	MergeType mergeType;
	int left, right /*, rLeft, lRight, rRight, height, size, distTBleft, distTBright*/;
	const DataType *label;

	friend ostream &operator<<(ostream &os, const Cluster &cluster) {
		os << "(" << cluster.left << "," << cluster.right << "/" << cluster.mergeType << "; ";
		if (cluster.label == NULL)
			os << "NULL";
		else
			os << *cluster.label;
		return os << ")";
	}
};

/// Top tree data structure
template <typename DataType>
struct TopTree {
	/// Create a top tree with labelled leaves
	/// \param numLeaves number of leaves in the top tree
	/// \param labels the labels to assign to the leaves
	TopTree(const int numLeaves, LabelsT<DataType> &labels) : clusters(numLeaves), numLeaves(numLeaves) {
		for (int i = 0; i < numLeaves; ++i) {
			clusters[i].label = &labels[i];
		}
	}

	/// Create a top tree with a fixed number of leaves
	/// \param numLeaves the number of leaves. Not terribly relevant, but other
	/// parts use this to do stuff [TM]
	TopTree(const int numLeaves) : clusters(numLeaves), numLeaves(numLeaves) {}

	/// Add a cluster to the top tree
	/// \param left left child
	/// \param right right child
	/// \param mergeType the type of merge that formed this cluster
	/// \return the new cluster's ID
	int addCluster(const int left, const int right, const MergeType mergeType) {
		clusters.push_back(Cluster<DataType>(left, right, mergeType));
		return clusters.size() - 1;
	}

	/// Traverse the tree in post order
	/// \param callback callback to call with the cluster ID as parameter
	void inPostOrder(const function<void(const int)> &callback) {
		traverseTreePostOrder(clusters.size() - 1, callback);
	}

	/// Helper for inPostOrder, you shouldn't need to use this
	void traverseTreePostOrder(const int clusterId, const function<void(const int)> &callback) {
		Cluster<DataType> &cluster = clusters[clusterId];
		if (cluster.left != -1) {
			traverseTreePostOrder(cluster.left, callback);
		}
		if (cluster.right != -1) {
			traverseTreePostOrder(cluster.right, callback);
		}
		callback(clusterId);
	}

	/// Traverse the top tree in post order, applying a callback to the callback results of its children
	/// \param callback function to be called on its results of the left and right child
	/// \param initial value to use as "callback result" for leaves
	template <typename T>
	T foldPostOrder(const function<T (const T, const T)> &callback, const T initial) const {
		return traverseFoldPostOrder(clusters.size() - 1, callback, initial);
	}

	/// Helper function for foldPostOrder(). You should not need to use this directly.
	template <typename T>
	T traverseFoldPostOrder(const int clusterId, const function<T (const T, const T)> &callback, const T initial) const {
		const Cluster<DataType> &cluster = clusters[clusterId];
		T left(initial), right(initial);
		if (cluster.left != -1) {
			left = traverseFoldPostOrder(cluster.left, callback, initial);
		}
		if (cluster.right != -1) {
			right = traverseFoldPostOrder(cluster.right, callback, initial);
		}
		return callback(left, right);
	}

	/// Get the height of the top tree
	int height() const {
		return foldPostOrder<int>([](const int leftHeight, const int rightHeight) {
			return std::max(leftHeight, rightHeight) + 1;
		}, -1);
	}

	double avgDepth() const {
		typedef std::pair<uint_fast32_t, uint_fast64_t> P;
		P countAndSum = foldPostOrder<P>([](const P left, const P right) {
			// there are numNodes(left) + numNodes(right) + 1 (=this) nodes in this subtree
			const auto count = left.first + right.first + 1;
			// each of their depths increases by 1, so add their count to the sum of total depth
			const auto sum = left.second + right.second + count;
			return P(count, sum);
		}, P(1, 0));
		return (double)countAndSum.second / countAndSum.first;
	}

	/// Check equality with another subtree
	bool isEqual(const TopTree<DataType> &other) const {
		if (numLeaves != other.numLeaves || clusters.size() != other.clusters.size()) {
			return false;
		}

		return nodesEqual(other, clusters.size() - 1, clusters.size() - 1);
	}

	/// Helper for isEqual, you shouldn't need to use this directly
	bool nodesEqual(const TopTree<DataType> &other, const int clusterId, const int otherClusterId) const {
		const Cluster<DataType> &cluster = clusters[clusterId];
		const Cluster<DataType> &otherCluster = other.clusters[otherClusterId];
		if ((cluster.label == NULL) != (otherCluster.label == NULL)) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (label NULL)" << endl;
			return false;
		}
		if (cluster.mergeType != otherCluster.mergeType) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (different merge types)" << endl;
			return false;
		}
		if ((cluster.left < 0) != (otherCluster.left < 0)) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (left < 0)" << endl;
			return false;
		}
		if ((cluster.right < 0) != (otherCluster.right < 0)) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (right < 0)" << endl;
			return false;
		}

		if (cluster.left >= 0 && !nodesEqual(other, cluster.left, otherCluster.left)) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (left clusters don't match)" << endl;
			return false;
		}
		if (cluster.right >= 0 && !nodesEqual(other, cluster.right, otherCluster.right)) {
			cout << "Difference in clusters " << clusterId << " / " << cluster << " and " << otherClusterId << " / "
				 << otherCluster << " (right clusters don't match)" << endl;
			return false;
		}
		return true;
	}

	friend ostream &operator<<(ostream &os, const TopTree<DataType> &toptree) {
		os << "Top tree with " << toptree.clusters.size() << " clusters. Non-leaves:" << endl;
		uint count = 0;
		for (const Cluster<DataType> &cluster : toptree.clusters) {
			if (cluster.left >= 0) {
				os << "\t" << count << ": " << cluster << endl;
			}
			count++;
		}
		return os;
	}

	vector<Cluster<DataType>> clusters;
	int numLeaves;
};

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
		int rootClusterId = topTree.clusters.size() - 1;

		Cluster<DataType> &rootCluster = topTree.clusters[rootClusterId];
		// special treatment for root node of original tree
		const DataType *label = topTree.clusters[rootCluster.left].label;
		assert(label != NULL);
		labels.set(0, *label);
		// unpack the rest
		unpackCluster(rootCluster.right, firstId);
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
