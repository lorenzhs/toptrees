#pragma once

#include "Common.h"
#include "Labels.h"
#include "OrderedTree.h"

/// Compute size of a succinct encoding of a tree
template <typename TreeType>
struct TreeSizeEstimation {
    /// Compute size of a succinct encoding of a tree
    /// \param tree the tree
    /// \param labels the nodes' labels
    /// \returns required size in bits
    static long long compute(const TreeType &tree, const Labels<std::string> &labels) {
        long long result(0);

        // space for label indices
        result += tree._numNodes * log_2(labels.size());

        // space for labels
        for (const std::string *label : labels.valueIndex) {
            // separate with a null-byte -> +1
            result += 8*(label->size() + 1);
        }

        // space for tree structure
        FORALL_NODES(tree, nodeId) {
            // Not coding the leaves
            if (tree.nodes[nodeId].hasChildren()) {
                result += 2;
            }
        }

        return result;
    }
};
