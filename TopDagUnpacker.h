#pragma once

#include <unordered_map>

#include "Common.h"
#include "TopDag.h"
#include "TopTree.h"


/// Unpack a binary DAG to its original top tree
template <typename DataType>
class TopDagUnpacker {
public:
    TopDagUnpacker(TopDag<DataType> &dag, TopTree<DataType> &topTree) : nextLeafId(0), dag(dag), topTree(topTree) {}

    void unpack() {
        assert((int)topTree.clusters.size() == topTree.numLeaves);
        dag.template inPostOrder<int>(
            [&](const int nodeId, const int left, const int right) {
                return addNodeToTopTree(nodeId, left, right);
            });
        // Restore the root label
        topTree.clusters[0].label = dag.nodes[1].label;
    }

private:
    int addNodeToTopTree(const int nodeId, const int left, const int right) {
        const DagNode<DataType> &node = dag.nodes[nodeId];
        assert((node.left < 0) == (node.right < 0));
        if (node.left < 0) {
            // it's a leaf
            assert(nextLeafId < topTree.numLeaves);
            int newId = nextLeafId++;
            topTree.clusters[newId].label = node.label;
            return newId;
        } else {
            // add `left` and `right` as children
            assert(node.label == NULL);
            assert(node.mergeType != NO_MERGE);
            return topTree.addCluster(left, right, node.mergeType);
        }
    }

    int nextLeafId;
    TopDag<DataType> &dag;
    TopTree<DataType> &topTree;
};
