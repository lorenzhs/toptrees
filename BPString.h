#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "Labels.h"
#include "OrderedTree.h"

struct BPString {
	enum Parentheses {OPEN = 0, CLOSE = 1};

	template <typename NodeType, typename EdgeType, typename DataType>
	static void fromTree(const OrderedTree<NodeType, EdgeType> &tree, const LabelsT<DataType> &labels, std::vector<bool> &bpstring, std::vector<unsigned char> &labelNames) {
		labelNames.clear();
		bpstring.clear();
		bpstring.reserve(2*tree._numNodes);

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
