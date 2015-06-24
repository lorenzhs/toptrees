/*
 * Applies top tree compression to an XML file. Can dump
 * the Top DAG as a DOT file, which is really the only
 * reason to use this anymore. You should probably look
 * at coding.cpp instead.
 */

#include <algorithm>
#include <iostream>
#include <string>

// Data structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "TopTree.h"

// Algorithms
#include "TopDagConstructor.h"
#include "TopTreeUnpacker.h"
#include "RePairCombiner.h"
#include "TopDagUnpacker.h"

// Utils
#include "ArgParser.h"
#include "DotGraphExporter.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;
	ArgParser argParser(argc, argv);
	const bool useRePair = argParser.isSet("r");
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	} else if (useRePair) {
		// if used as "./test -r foo.xml", it will match the foo.xml to the "-r" which is unfortunate
		string arg = argParser.get<string>("r", "");
		filename = (arg == "") ? filename : arg;
	}
	const bool writeDotFiles = argParser.isSet("w");

	Labels<string> labels;

	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	cout << t.summary() << "; Height: " << t.height() << " Avg depth: " << t.avgDepth() << endl;

	if (writeDotFiles) {
		OrderedTreeDotGraphExporter<TreeNode, TreeEdge, string>().write(t, labels, "/tmp/tree.dot");
	}

	const int treeEdges = t._numEdges;
	TopDag<string> topDag(t._numNodes, labels);

	Timer timer;
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, topDag);
		topDagConstructor.construct();
	} else {
		TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, topDag);
		topDagConstructor.construct();
	}
	cout << "Top DAG construction took " << timer.getAndReset() << "ms" << endl;
	//, avg node depth " << topTree.avgDepth() << " (min " << topTree.minDepth() << "); took " << timer.getAndReset() << " ms" << endl;

	const int edges = topDag.countEdges();
	const double percentage = (edges * 100.0) / treeEdges;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << topDag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl;

	if (writeDotFiles) {
		TopDagDotGraphExporter<string>().write(topDag, "/tmp/topdag.dot");
		DotGraphExporter<TopDag<string>>::drawSvg("/tmp/topdag.dot", "/tmp/topdag.svg");
	}

	return 0;
}
