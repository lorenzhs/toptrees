#pragma once

#include <cassert>
#include <stack>

#include "TopDag.h"

/// Represents an entry in the DAG stack
struct NavigationRecord {
    int nodeId;
    int parentId;
    bool left;
    /// Create a blank navigation record
    NavigationRecord() : nodeId(-1), parentId(-1), left(true) {}
    /// Create a new navigation record
    /// \param node node to which we moved
    /// \param parent node from which we came
    /// \param left whether we went into `parent`'s left child to get to `node`
    NavigationRecord(int node, int parent, bool left) : nodeId(node), parentId(parent), left(left) {}

    friend std::ostream &operator<<(std::ostream &os, const NavigationRecord &record) {
        return os << "(" << record.nodeId << ";" << record.parentId << ";" << record.left << ")";
    }
};

/// Navigate around in an in-memory Top DAG
template <typename DataType>
class Navigator {
public:
    using DAGType = TopDag<DataType>;
    using DStackT = std::stack<NavigationRecord>;
    using TStackT = std::deque<DStackT>;

    /// Create a new navigator for the given Top DAG
    Navigator(const DAGType &dag): dag(dag), dagStack(), treeStack(), maxTreeStackSize(0) {
        if (verbose) std::cout << dag << std::endl;

        int nodeId = -1;
        int nextNode = (int)dag.nodes.size() - 1;
        while (nextNode > 0) {
            dagStack.emplace(NavigationRecord{nextNode, nodeId, true});
            nodeId = nextNode;
            nextNode = dag.nodes[nodeId].left;
        }
    }

    /// Retrieve the current node's label
    const DataType* getLabel() const {
        return dag.nodes[dagStack.top().nodeId].label;
    }

    /// Move to the current node's parent
    /// \returns whether the operation completed successfully (i.e. if the
    /// current node had a parent)
    bool parent() {
        if (treeStack.empty()) {
            return false;
        } else {
            dagStack = treeStack.back();
            treeStack.pop_back();
            return true;
        }
    }

    /// Check whether the current node is a leaf in the tree
    bool isLeaf() {
        DStackT stack(dagStack);
        while (!stack.empty()) {
            NavigationRecord &record = stack.top();
            MergeType mergeType = dag.nodes[record.parentId].mergeType;

            if ((!record.left && (mergeType == VERT_NO_BBN ||
                                  mergeType == HORZ_LEFT_BBN)) || // b or c from the right
                (record.left && mergeType == HORZ_RIGHT_BBN) || // d from the left
                mergeType == HORZ_NO_BBN ||  // type e, either side
                (record.nodeId == (int)dag.nodes.size() - 1 && !record.left))  // reached root from the right
            {
                return true;
            }

            if (record.left && (mergeType == VERT_WITH_BBN ||
                                mergeType == VERT_NO_BBN)) {
                // vertical merge from the left/top
                return false;
            }
            stack.pop();
        }
        assert(false);
        return false;
    }

    /// Move to the current node's first child
    /// \returns whether the operation was a success
    bool firstChild() {
        if (verbose) dumpDagStack();
        if (isLeaf()) {
            return false;
        }
        treeStack.push_back(dagStack);
        maxTreeStackSize = std::max(maxTreeStackSize, getTreeStackSize());
        while (!dagStack.empty()) {
            NavigationRecord &record = dagStack.top();
            MergeType mergeType = dag.nodes[record.parentId].mergeType;

            if (record.left && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
                break;
            }
            dagStack.pop();
        }
        auto nodeId(dagStack.top().parentId), nextNode(dag.nodes[nodeId].right);
        dagStack.pop();
        dagStack.emplace(NavigationRecord{nextNode, nodeId, false});
        if (verbose) std::cout << "fC: pushing " << dagStack.top() << std::endl;

        while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
            dagStack.emplace(NavigationRecord{nextNode, nodeId, true});
            if (verbose) std::cout << "fC: pushing " << dagStack.top() << std::endl;
        }
        return true;
    }

    /// Move to the current node's last child
    /// \returns whether the operation was a success
    bool lastChild() {
        if (verbose) dumpDagStack();
        if (isLeaf()) {
            return false;
        }
        treeStack.push_back(dagStack);
        maxTreeStackSize = std::max(maxTreeStackSize, getTreeStackSize());

        while (!dagStack.empty()) {
            NavigationRecord &record = dagStack.top();
            MergeType mergeType = dag.nodes[record.parentId].mergeType;

            if (record.left && (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN)) {
                break;
            }
            dagStack.pop();
        }
        auto nodeId(dagStack.top().parentId), nextNode(dag.nodes[nodeId].right);
        bool wentLeft = false;
        dagStack.pop();

        while (nextNode > 0) {
            dagStack.emplace(NavigationRecord{nextNode, nodeId, wentLeft});
            if (verbose) std::cout << "lC: pushing " << dagStack.top() << std::endl;
            nodeId = nextNode;
            auto mergeType = dag.nodes[nextNode].mergeType;
            if (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN) {
                nextNode = dag.nodes[nextNode].left;
                wentLeft = true;
            } else {
                nextNode = dag.nodes[nextNode].right;
                wentLeft = false;
            }
        }

        return true;
    }

    /// Move to the current node's next sibling
    /// \returns whether the operation was a success
    bool nextSibling() {
        if (verbose) dumpDagStack();

        DStackT stack(dagStack);
        while (!stack.empty()) {
            NavigationRecord &record = stack.top();
            MergeType mergeType = dag.nodes[record.parentId].mergeType;
            if (record.left && (mergeType == HORZ_LEFT_BBN ||
                                mergeType == HORZ_RIGHT_BBN ||
                                mergeType == HORZ_NO_BBN)) {
                break;
            }
            if ((!record.left) && (mergeType == VERT_WITH_BBN ||
                                   mergeType == VERT_NO_BBN)) {
                // a or b from right => abort
                return false;
            }
            stack.pop();
        }

        // No more next siblings in the tree, we've exhausted the stack
        if (stack.empty()) {
            return false;
        }
        dagStack = stack;

        auto nodeId(dagStack.top().parentId), nextNode(dag.nodes[nodeId].right);
        dagStack.pop();
        dagStack.emplace(NavigationRecord{nextNode, nodeId, false});
        if (verbose) std::cout << "nS: pushing " << dagStack.top() << std::endl;

        while ((nodeId = nextNode) > 0 && (nextNode = dag.nodes[nextNode].left) > 0) {
            dagStack.emplace(NavigationRecord{nextNode, nodeId, true});
            if (verbose) std::cout << "nS: pushing " << dagStack.top() << std::endl;
        }
        return true;
    }


    /// Move to the current node's previous sibling
    /// \returns whether the operation was a success
    bool prevSibling() {
        if (verbose) dumpDagStack();

        DStackT stack(dagStack);
        while (!stack.empty()) {
            NavigationRecord &record = stack.top();
            MergeType mergeType = dag.nodes[record.parentId].mergeType;
            if (!record.left && (mergeType == HORZ_LEFT_BBN ||
                                 mergeType == HORZ_RIGHT_BBN ||
                                 mergeType == HORZ_NO_BBN)) {
                break;
            }
            if ((!record.left) && (mergeType == VERT_WITH_BBN ||
                                   mergeType == VERT_NO_BBN)) {
                // a or b from right => abort
                return false;
            }
            stack.pop();
        }

        // No more next siblings in the tree, we've exhausted the stack
        if (stack.empty()) {
            return false;
        }

        dagStack = stack;
        auto nodeId(dagStack.top().parentId), nextNode(dag.nodes[nodeId].left);
        bool wentLeft = true;
        dagStack.pop();

        while (nextNode > 0) {
            dagStack.emplace(NavigationRecord{nextNode, nodeId, wentLeft});
            if (verbose) std::cout << "pS: pushing " << dagStack.top() << std::endl;
            nodeId = nextNode;
            auto mergeType = dag.nodes[nextNode].mergeType;
            if (mergeType == VERT_WITH_BBN || mergeType == VERT_NO_BBN) {
                nextNode = dag.nodes[nextNode].left;
                wentLeft = true;
            } else {
                nextNode = dag.nodes[nextNode].right;
                wentLeft = false;
            }
        }

        return true;
    }

    /// Debug helper to dump the DAG stack
    void dumpDagStack() {
        DStackT stack(dagStack);
        std::cout << "DagStack: ";
        while (!stack.empty()) {
            std::cout << stack.top() << " :: ";
            stack.pop();
        }
        std::cout << std::endl;
    }

    /// Debug helper to retrieve tree stack size
    unsigned long long getTreeStackSize() const {
        unsigned long long treeStackSize(0);
        for (const DStackT &stack : treeStack) {
            treeStackSize += stack.size();
        }
        treeStackSize *= sizeof(DStackT::value_type);
        return treeStackSize;
    }

    /// Debug helper to return largest tree stack size encountered
    unsigned long long getMaxTreeStackSize() const {
        return maxTreeStackSize;
    }

private:
    const DAGType &dag;
    DStackT dagStack;
    TStackT treeStack;
    unsigned long long maxTreeStackSize;
    static const bool verbose = false;
};
