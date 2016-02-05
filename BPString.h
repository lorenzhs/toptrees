#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "Labels.h"
#include "OrderedTree.h"

/// Convert tree to BP string & label char collection
struct BPString {
    enum Parentheses {OPEN = 0, CLOSE = 1};

    /// Turn an OrderedTree instance into a balanced parenthesis
    /// bitstring for the tree structure, and a nullbyte-separated
    /// label vector (null-bytes not allowed in input labels).
    /// A node is coded as "(children)", e.g. "(()())" for a node with
    /// two leaf children
    template <typename NodeType, typename EdgeType, typename DataType>
    static void fromTree(const OrderedTree<NodeType, EdgeType> &tree, const LabelsT<DataType> &labels, std::vector<bool> &bpstring, std::vector<unsigned char> &labelNames) {
        labelNames.clear();
        bpstring.clear();
        bpstring.reserve(2*tree._numNodes);

        // Recursive construction of the output
        const std::function<void (const int)> parseStructure([&](const int nodeId) {
            const auto &label(labels[nodeId]);
            std::copy(label.cbegin(), label.cend(), std::back_inserter(labelNames));
            labelNames.push_back(0);

            bpstring.push_back(OPEN);

            FORALL_OUTGOING_EDGES(tree, nodeId, edge) {
                if (edge->valid) {
                    parseStructure(edge->headNode);
                }
            }

            bpstring.push_back(CLOSE);
        });

        parseStructure(0);
    };
};
