#pragma once

#include "Common.h"
#include "Labels.h"
#include "OrderedTree.h"

template <typename TreeType>
struct TreeSizeEstimation {
	static long long compute(const TreeType &tree, const Labels<std::string> &labels) {
		long long result(0);

		// space for label indices
		result += tree._numNodes * log2(labels.size());

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
