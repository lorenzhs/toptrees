#include <algorithm>
#include <iostream>
#include <string>

// Data structures
#include "BinaryDag.h"
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopTree.h"

// Algorithms
#include "TopTreeConstructor.h"
#include "TopTreeUnpacker.h"
#include "RePairCombiner.h"
#include "DagBuilder.h"

// Utils
#include "ArgParser.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;
//*
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

	Labels<string> labels;

	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	cout << t.summary() << "; Height: " << t.height() << " Avg depth: " << t.avgDepth() << endl;

	TopTree<string> topTree(t._numNodes, labels);

	Timer timer;
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, string> topTreeConstructor(t, topTree, labels);
		topTreeConstructor.construct();
	} else {
		TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, string> topTreeConstructor(t, topTree);
		topTreeConstructor.construct();
	}
	cout << "Top tree construction took " << timer.getAndReset() << "ms, avg node depth " << topTree.avgDepth() << " (min " << topTree.minDepth() << "); took " << timer.getAndReset() << " ms" << endl;

	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl
		 << "Top dag construction took in " << timer.get() << "ms" << endl;

/*/

	(void) argc;
	(void) argv;

	// vector<int> edges({0,1, 1,2, 1,3, 2,4, 2,5, 2,6, 5,7, 3,8, 3,9, 3,10, 9,11});

	vector<int> edges({0,1, 0,2, 0,3, 1,4, 1,5, 4,9, 4,10, 3,6, 6,7, 7,8});
	const int numNodes = *std::max_element(edges.begin(), edges.end()) + 1;
	t.addNodes(numNodes);
	for (uint i = 0; i < edges.size(); i += 2) {
		t.addEdge(edges[i], edges[i+1]);
	}

	cout << t << endl << endl;

	FakeLabels<int> labels(0);

	t.toString();

	TopTree<int> topTree(numNodes, labels);

	//RePairCombiner<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(t, topTree, labels, true, true);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(t, topTree, true, true);
	topTreeConstructor.construct(NULL);

	cout << endl << t << endl;
	cout << topTree << endl;
	BinaryDag<int> dag;
	DagBuilder<int> builder(topTree, dag);
	builder.createDag();
	cout << dag << endl;

	const int numEdges = dag.countEdges();
	const double percentage = (numEdges * 100.0) / (edges.size()/2);
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << numEdges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl;

	XmlWriter<TopTree<int>>::write(topTree, "/tmp/toptree.xml");

	TopTree<int> newTopTree(numNodes);
	BinaryDagUnpacker<int> dagUnpacker(dag, newTopTree);
	dagUnpacker.unpack();
	//cout << newTopTree << endl;

	OrderedTree<TreeNode, TreeEdge> newTree;
	Labels<int> newLabels;
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, int> unpacker(topTree, newTree, newLabels);
	unpacker.unpack();

	XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(newTree, newLabels, "/tmp/unpacked.xml");

//*/
	return 0;
}
