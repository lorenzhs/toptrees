/*
 * Tests top tree compression on an XML file
 *
 * Compresses and decompresses the input file, and
 * checks whether they match.
 */

#include <iostream>
#include <string>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "XML.h"
#include "Timer.h"

#include "TopDagUnpacker.h"
#include "TopDagConstructor.h"
#include "RePairCombiner.h"
#include "TopTreeUnpacker.h"

#include "ArgParser.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;

	Labels<string> labels(0);

	ArgParser argParser(argc, argv);
	const bool useRePair = argParser.isSet("r");
	string filename = argParser.get<string>("i", "data/1998statistics.xml");
	string outputfolder = argParser.get<string>("o", "/tmp");

	// Read input file
	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	// Dump input file for comparison of output
	Timer timer;
	XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(t, labels, outputfolder + "/orig.xml");

	cout << "Wrote orginial trimmed XML file in " << timer.getAndReset() << "ms: " << t.summary() << endl;

	// Prepare for construction of top tree
	const int size = t._numNodes;
	TopDag<string> dag(size, labels);
	timer.reset();

	// construct top tree
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct();
	} else {
		TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct();
	}

	t.clear();

	cout << "Top DAG construction took " << timer.getAndReset() << "ms" << endl;
	//", avg node depth " << topTree.avgDepth() << " (min " << topTree.minDepth() << "); took " << timer.getAndReset() << " ms" << endl;

	cout << "Top DAG has " << dag.nodes.size() - 1 << " nodes, " << dag.countEdges() << " edges" << endl;

	// Unpack top DAG to recoveredTopTree
	TopTree<string> recoveredTopTree(size);
	TopDagUnpacker<string> dagUnpacker(dag, recoveredTopTree);
	dagUnpacker.unpack();

	cout << "Unpacked Top DAG in " << timer.getAndReset() << "ms, top tree has " << recoveredTopTree.clusters.size() << " clusters" << endl;

	// unpack recovered top tree
	OrderedTree<TreeNode, TreeEdge> recoveredTree;
	Labels<string> newLabels(labels.numKeys());
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, string> unpacker(recoveredTopTree, recoveredTree, newLabels);
	unpacker.unpack();
	cout << "Unpacked recovered top tree in " << timer.getAndReset() << "ms: " << recoveredTree.summary() << endl;

	XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(recoveredTree, newLabels, outputfolder + "/unpacked.xml");
	cout << "Wrote recovered tree in " << timer.getAndReset() << "ms" << endl;

	return 0;
}
