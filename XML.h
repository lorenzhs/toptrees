#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "Timer.h"
#include "OrderedTree.h"
#include "TopTree.h"
#include "Labels.h"

#include "3rdparty/rapidxml.hpp"
#include "3rdparty/rapidxml_utils.hpp"

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
	static void parse(const string &filename, TreeType &tree, Labels<string> &labels, const bool verbose = true) {
		// recursively parse the XML tree
		const std::function<void (rapidxml::xml_node<>*, const int)> parseStructure([&](rapidxml::xml_node<> *node, const int id) {
			rapidxml::xml_node<> *child = node->first_node();
			int numChildren(0), childId;
			// Add the children before processing them
			// Otherwise, the edges would have to be moved all the time
			// in the tree because of the adjacency array data structure
			while (child != NULL) {
				numChildren++;
				childId = tree.addNode();
				tree.addEdge(id, childId);
				labels.set(childId, child->name());
				child = child->next_sibling();
			}

			if (numChildren == 0) return;
			childId -= numChildren - 1;
			child = node->first_node();
			// recurse to the children
			while (child != NULL) {
				parseStructure(child, childId);
				childId++;
				child = child->next_sibling();
			}
		});


		if (verbose) cout << "Reading " << filename << "… " << flush;
		Timer timer;

		rapidxml::file<> file(filename.c_str());
		rapidxml::xml_document<> xml;
		if (verbose) cout << " " << timer.getAndReset() << "ms; parsing… " << flush;

		xml.parse<rapidxml::parse_no_data_nodes>(file.data());
		rapidxml::xml_node<> *root = xml.first_node();
		if (verbose) cout << timer.getAndReset() << "ms; building tree… " << flush;

		const int rootId = tree.addNode();
		assert((int)labels.size() == rootId);
		labels.set(rootId, root->name());
		parseStructure(root, rootId);

		if (verbose) cout << timer.get() << "ms." << endl;
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

		const std::function<void (const int, const int)> writeNode([&] (const int nodeId, const int depth) {
			const Cluster<DataType> &node = tree.clusters[nodeId];
			for (int i = 0; i < depth; ++i) out << " ";
			out << "<";
			if (node.label == NULL) out << node.mergeType; else out << *node.label;
			out << ">";

			if (node.left >= 0 || node.right >= 0) {
				out << endl;

				if (node.left >= 0) {
					writeNode(node.left, depth + 1);
				}
				if (node.right >= 0) {
					writeNode(node.right, depth + 1);
				}

				for (int i = 0; i < depth; ++i) out << " ";
			}

			out << "</";
			if (node.label == NULL) out << node.mergeType; else out << *node.label;
			out << ">";
		});

		int rootId = tree.clusters.size() - 1;
		writeNode(rootId, 0);

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

		const std::function<void (const int, const int)> writeNode([&](const int nodeId, const int depth) {
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
					writeNode(edge->headNode, depth + 1);
			}

			if (indent) for (int i = 0; i < depth; ++i) out << " ";
			out << "</" << labels[nodeId] << ">";
			if (indent) out << endl;
		});

		writeNode(0, 0);

		out.close();
	}
};
