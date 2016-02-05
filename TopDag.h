#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

#include "Labels.h"
#include "Nodes.h"

using std::vector;


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


/// A binary DAG that is specialised to be a top tree's minimal DAG
template <typename DataType>
class TopDag {
public:
    /// Create a new binary DAG
    TopDag(const size_t n, const LabelsT<DataType> &labels) :
        maxClusterId(n-1),
        nodes(),
        labels(labels),
        nodeMap(),
        clusterToDag(2*n, -1) // TODO check number
    {
        // add a dummy element that is guaranteed to not appear
        // (we assume that -1 is used for leaves, never -2, except for this dummy)
        nodes.emplace_back(-2, -2, (DataType *)NULL, NO_MERGE);

        // Add the leaves
        for (size_t i = 0; i < n; ++i) {
            clusterToDag[i] = addCluster_(-1, -1, NO_MERGE, &labels[i]);
        }

        //std::cout << "TD: added " << n  << " leaves, " << nodes.size() << " = "
        //        << nodeMap.size() + 1 << " of which are distinct" << std::endl;
    }

    /// Add a cluster and return its cluster ID (!= node ID!).
    /// If the cluster already exists in the DAG, it is not created again.
    /// \param left cluster ID of the left child cluster
    /// \param right cluster ID of the right child cluster
    /// \param mergeType the cluster's merge type
    /// \param label the cluster's label (if any)
    int addCluster(int left, int right, const MergeType mergeType, const DataType *label = NULL) {
        const int nodeId = addCluster_(left, right, mergeType, label);
        clusterToDag[++maxClusterId] = nodeId;
        return maxClusterId;
    }

    /// Call this to clean up temporary data structures once the DAG is final
    void finishCreation() {
        nodeMap.clear();
    }

    /// Count the number of edges in the DAG
    int countEdges() const {
        int count(0);
        for (const DagNode<DataType> &node : nodes) {
            count += (node.left >= 0);
            count += (node.right >= 0);
        }
        return count;
    }

    /// Traverse the dag in post-order
    /// \param callback a callback to be called with the node ID and the results of the calls to its children
    template <typename T, typename Callback>
    T inPostOrder(const Callback &callback) const {
        return traverseDagPostOrder<T, Callback>(nodes.size() - 1, callback);
    }

    /// Helper for inPostOrder(), you shouldn't need to call this directly
    template <typename T, typename Callback>
    T traverseDagPostOrder(const int nodeId, const Callback &callback) const {
        assert(nodeId != 0); // 0 is the dummy not and should not be reachable
        const DagNode<DataType> &node = nodes[nodeId];
        T left(-1), right(-1);
        if (node.left >= 0) {
            left = traverseDagPostOrder<T, Callback>(node.left, callback);
        }
        if (node.right >= 0) {
            right = traverseDagPostOrder<T, Callback>(node.right, callback);
        }
        return callback(nodeId, left, right);
    }

    friend std::ostream &operator<<(std::ostream &os, const TopDag<DataType> &dag) {
        os << "Binary Dag with " << dag.nodes.size() - 1 << " nodes";
        for (uint i = 1; i < dag.nodes.size(); ++i) {
            os << "; " << i << "=" << dag.nodes[i];
        }
        return os;
    }

protected:
    /// Add a node
    int addCluster_(int left, int right, const MergeType mergeType, const DataType *label = NULL) {
        assert((left < 0) == (right < 0));
        if (left >= 0) {
            left = clusterToDag[left];
            right = clusterToDag[right];
            assert(left > 0 && right > 0);
        }
        DagNode<DataType> node(left, right, label, mergeType);

        //std::cout << "TD: adding node " << node << std::flush;
        int &id = nodeMap[node];
        if (id == 0) {
            // node is new
            nodes.push_back(node);
            id = nodes.size() - 1;
            // Increase the childrens' in-degree
            if (left >= 0) nodes[left].inDegree++;
            if (right >= 0) nodes[right].inDegree++;
            //std::cout << " (new node)";
        }
        //std::cout << " ID=" << id << std::endl;
        return id;
    }

    /// Add a node
    /// \param left left child
    /// \param right right child
    /// \param label a label pointer
    /// \param mergeType the original node's merge type (for use with a TopTree)
    int addNode(int left, int right, MergeType mergeType, const DataType *label) {
        nodes.emplace_back(left, right, label, mergeType);
        return (nodes.size() - 1);
    }

    /// Remove the last node
    void popNode() {
        nodes.pop_back();
    }

    /// Add multiple nodes
    /// \param n the number of nodes to add
    /// \return the ID of the first node added
    int addNodes(const int n) {
        nodes.resize(nodes.size() + n);
        return nodes.size() - n;
    }

public:
    int maxClusterId;
    vector<DagNode<DataType>> nodes;
    const LabelsT<DataType> &labels;
    std::unordered_map<DagNode<DataType>, int, SubtreeHasher<DataType>, SubtreeEquality<DataType>> nodeMap;
    vector<int> clusterToDag;
};
