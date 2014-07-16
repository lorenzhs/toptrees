#pragma once

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>

using std::string;

/// Export a tree as a DOT graph
struct DotGraphExporter {
	template <typename TreeType>
	/// write a tree's dot graph to a file
	/// \param tree the tree to write
	/// \param filename output filename (path must exist)
	static void write(const TreeType &tree, const string &filename) {
		std::ofstream out(filename);
		assert(out.is_open());
		out << "digraph myTree {" << std::endl;
		writeNode(out, tree, 0);
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
	/// iteratively write the tree to an output stream
	template <typename TreeType>
	static void writeNode(std::ostream &out, const TreeType &tree, const int nodeId) {
		FORALL_OUTGOING_EDGES(tree, nodeId, edge) {
			if (!edge->valid) continue;
			out << "\t" << nodeId << " -> " << edge->headNode << ";" << std::endl;
			writeNode(out, tree, edge->headNode);
		}
	}
};