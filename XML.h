#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "Timer.h"
#include "OrderedTree.h"
#include "TopTree.h"
#include "Labels.h"

#include "3rdparty/pugixml.hpp"


using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

/// Read an XML file into a tree, using RapidXml
template <typename TreeType>
struct XmlParser {
	// TODO figure out if we can keep the char pointers instead of converting them
	// to string, this currently uses more than half of the parsing time
	static bool parse(const string &filename, TreeType &tree, Labels<string> &labels, const bool verbose = true) {
		if (verbose) cout << "Reading and parsing " << filename << "… " << flush;
		Timer timer;

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_minimal);
		if (!result) { // parse failed
			return false;
		}

		if (verbose) cout << timer.getAndReset() << "ms; building tree… " << flush;

		const int rootId = tree.addNode();
		assert((int)labels.size() == rootId);

		pugi::xml_node root(doc.root().first_child());
		labels.set(rootId, root.name());
		parseStructure(tree, labels, root, rootId);

		if (verbose) cout << timer.get() << "ms." << endl;
		return true;
	}

protected:
	static void parseStructure(TreeType &tree, Labels<string> &labels, pugi::xml_node node, const int id) {
		const size_t numChildren = std::distance(node.children().begin(), node.children().end());
		int childId = tree.addNodes(numChildren);
		for (size_t i = 0; i < numChildren; ++i) {
			tree.addEdge(id, childId + i);
		}

		// Recurse into children
		for (pugi::xml_node child : node.children()) {
			labels.set(childId, child.name());
			parseStructure(tree, labels, child, childId);
			++childId;
		}
	}
};

/// XML tree writer (empty template for overloading)
template <typename TreeType>
struct XmlWriter {};

/// Top tree XML writer
template <typename DataType>
struct XmlWriter<TopTree<DataType>> {
	/// Write a TopTree instance to an XML file.
	/// Nodes without labels will have their merge types used as labels
	/// \param tree the TopTree to write
	/// \param filename output filename (path must exist)
	static void write(const TopTree<DataType> &tree, const string &filename) {
		std::ofstream out(filename.c_str());
		assert(out.is_open());

		auto writeNode = [&] (const int nodeId, const int depth, const auto &writeNode) {
			const Cluster<DataType> &node = tree.clusters[nodeId];
			for (int i = 0; i < depth; ++i) out << " ";
			out << "<";
			if (node.label == NULL) out << node.mergeType; else out << *node.label;
			out << ">";

			if (node.left >= 0 || node.right >= 0) {
				out << endl;

				if (node.left >= 0) {
					writeNode(node.left, depth + 1, writeNode);
				}
				if (node.right >= 0) {
					writeNode(node.right, depth + 1, writeNode);
				}

				for (int i = 0; i < depth; ++i) out << " ";
			}

			out << "</";
			if (node.label == NULL) out << node.mergeType; else out << *node.label;
			out << ">";
		};

		int rootId = tree.clusters.size() - 1;
		writeNode(rootId, 0, writeNode);

		out.close();
	}
};

/// OrderedTree XML tree writer
template <typename NodeType, typename EdgeType>
struct XmlWriter<OrderedTree<NodeType, EdgeType>> {
	/// write an OrderedTree to an XML file, using its labels
	/// \param tree the OrderedTree instance to write
	/// \param labels the nodes' labels
	/// \param filename filename to use. Directory must exist.
	template <typename DataType>
	static void write(const OrderedTree<NodeType, EdgeType> &tree, const LabelsT<DataType> &labels, const string &filename, const bool indent=true) {
		std::ofstream out(filename.c_str());
		assert(out.is_open());

		auto writeNode = [&] (const int nodeId, const int depth, const auto &writeNode) {
			if (indent) for (int i = 0; i < depth; ++i) out << " ";
			out << "<" << labels[nodeId] << ">";
			if (tree.nodes[nodeId].isLeaf()) {
				out << "</" << labels[nodeId] << ">";
				if (indent) out << endl;
				return;
			}
			if (indent) out << endl;

			FORALL_OUTGOING_EDGES(tree, nodeId, edge) {
				if (edge->valid)
					writeNode(edge->headNode, depth + 1, writeNode);
			}

			if (indent) for (int i = 0; i < depth; ++i) out << " ";
			out << "</" << labels[nodeId] << ">";
			if (indent) out << endl;
		};

		writeNode(0, 0, writeNode);

		out.close();
	}
};
