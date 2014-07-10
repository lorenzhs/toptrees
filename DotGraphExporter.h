#pragma once

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>

using std::endl;
using std::ofstream;
using std::string;
using std::stringstream;

struct DotGraphExporter {
	template <typename TreeType>
	static void write(const TreeType &tree, const string &filename) {
		ofstream out(filename);
		assert(out.is_open());
		out << "digraph myTree {" << endl;
		writeNode(out, tree, 0);
		out << "}" << endl;
	}

	// UNSAFE
	static void drawSvg(const string &dotfile, const string &outfilename) {
		stringstream s;
		s << "dot -Tsvg " << dotfile << " -o " << outfilename;
		system(s.str().c_str());
	}

private:
	template <typename TreeType>
	static void writeNode(ofstream &out, const TreeType &tree, const int nodeId) {
		FORALL_OUTGOING_EDGES(tree, nodeId, edge) {
			if (!edge->valid) continue;
			out << "\t" << nodeId << " -> " << edge->headNode << ";";
			writeNode(out, tree, edge->headNode);
		}
	}
};