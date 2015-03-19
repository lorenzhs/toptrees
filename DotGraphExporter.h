#pragma once

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "TopDag.h"
#include "OrderedTree.h"
#include "TopTree.h"

using std::string;

template <typename TreeType>
struct DotGraphExporter {
	/// write a tree's dot graph to a file
	/// \param tree the tree to write
	/// \param filename output filename (path must exist)
	void write(const TreeType &tree, const string &filename, const int nodeId = 0) {
		std::ofstream out(filename);
		assert(out.is_open());
		out << "digraph myTree {" << std::endl;
		writeNode(out, tree, nodeId);
		out << "}" << std::endl;
	}

	// UNSAFE
	/// plot a dotfile to svg using the utterly unsafe system() function
	/// \param dotfile filename of the dotfile
	/// \param outfilename filename of the svg file to be generated
	static void drawSvg(const string &dotfile, const string &outfilename) {
		std::stringstream s;
		s << "dot -Tsvg " << dotfile << " -o " << outfilename;
		system(s.str().c_str());
	}
protected:
	virtual void writeNode(std::ostream&, const TreeType&, const int) = 0;
};

/// Export a tree as a DOT graph
template <typename NodeType, typename EdgeType, typename DataType>
struct OrderedTreeDotGraphExporter {
	/// write a tree's dot graph to a file
	/// \param tree the tree to write
	/// \param filename output filename (path must exist)
	void write(const OrderedTree<NodeType, EdgeType> &tree,  const LabelsT<DataType> &labels, const string &filename, const int nodeId = 0) {
		std::ofstream out(filename);
		assert(out.is_open());
		out << "digraph myTree {" << std::endl;
		writeNode(out, tree, labels, nodeId);
		out << "}" << std::endl;
	}
protected:
	/// iteratively write the tree to an output stream
	void writeNode(std::ostream &out, const OrderedTree<NodeType, EdgeType> &tree, const LabelsT<DataType> &labels, const int nodeId) {
		out << "\t" << nodeId << " [label=\"" << nodeId << "/" << labels[nodeId] << "\"]" << std::endl;
		FORALL_OUTGOING_EDGES(tree, nodeId, edge) {
			if (!edge->valid) continue;
			out << "\t" << nodeId << " -> " << edge->headNode << ";" << std::endl;
			writeNode(out, tree, labels, edge->headNode);
		}
	}
};

template <typename DataType>
struct TopTreeDotGraphExporter : DotGraphExporter<TopTree<DataType>> {
protected:
	/// iteratively write the tree to an output stream
	void writeNode(std::ostream &out, const TopTree<DataType> &tree, const int clusterId) {
		const auto &cluster = tree.clusters[clusterId];
		if (cluster.label != NULL) {
			out << "\t" << clusterId << " [label=\"" << clusterId << "/" << *cluster.label << "\"]" << std::endl;
		} else {
			out << "\t" << clusterId << " [label=\"" << clusterId << ";" << cluster.mergeType << "\"]" << std::endl;
		}
		if (cluster.left >= 0) {
			out << "\t" << clusterId << " -> " << cluster.left << ";" << std::endl;
			writeNode(out, tree, cluster.left);
		}
		if (cluster.right >= 0) {
			out << "\t" << clusterId << " -> " << cluster.right << ";" << std::endl;
			writeNode(out, tree, cluster.right);
		}
	}
};


template <typename DataType>
struct TopDagDotGraphExporter : DotGraphExporter<TopDag<DataType>> {
	/// write a tree's dot graph to a file
	/// \param tree the tree to write
	/// \param filename output filename (path must exist)
	void write(const TopDag<DataType> &dag, const string &filename) {
		alreadyProcessed.assign(dag.nodes.size(), false);

		std::ofstream out(filename);
		assert(out.is_open());
		out << "digraph myTree {" << std::endl;
		writeNode(out, dag, dag.nodes.size() - 1);
		out << "}" << std::endl;
	}
protected:
	/// iteratively write the tree to an output stream
	void writeNode(std::ostream &out, const TopDag<DataType> &dag, const int nodeId) {
		if (alreadyProcessed[nodeId]) return;
		alreadyProcessed[nodeId] = true;
		const auto &node = dag.nodes[nodeId];
		if (node.label != NULL) {
			out << "\t" << nodeId << " [label=\"" << nodeId << "/" << *node.label << "\"]" << std::endl;
		} else {
			out << "\t" << nodeId << " [label=\"" << nodeId << ";" << node.mergeType << "\"]" << std::endl;
		}
		if (node.left >= 0) {
			out << "\t" << nodeId << " -> " << node.left << ";" << std::endl;
			writeNode(out, dag, node.left);
		}
		if (node.right >= 0) {
			out << "\t" << nodeId << " -> " << node.right << ";" << std::endl;
			writeNode(out, dag, node.right);
		}
	}

	vector<bool> alreadyProcessed;
};
