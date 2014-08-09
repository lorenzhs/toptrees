#include <iostream>
#include <vector>

// Data structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopTree.h"

// Algorithms
#include "TopTreeConstructor.h"
#include "RePairCombiner.h"
#include "DagBuilder.h"

// Utils
#include "ArgParser.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;
using std::vector;

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
	cout << "Top tree construction took " << timer.getAndReset() << "ms" << endl;

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

	int foo = argc;
	foo++;
	char** bar = argv;
	bar++;

	t.addNodes(12);
	t.addEdge(0, 1);
	t.addEdge(1, 2);
	t.addEdge(1, 3);
	t.addEdge(2, 4);
	t.addEdge(2, 5);
	t.addEdge(2, 6);
	t.addEdge(5, 7);
	t.addEdge(3, 8);
	t.addEdge(3, 9);
	t.addEdge(3, 10);
	t.addEdge(9, 11);
	cout << t << endl << endl;

	FakeLabels<int> labels(0);

	t.toString();

	TopTree<int> topTree(t._numNodes, labels);

	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(t, topTree);
	topTreeConstructor.construct(NULL, true, true);

	cout << endl << t << endl;
	//cout << topTree << endl;
	BinaryDag<int> dag;
	DagBuilder<int> builder(topTree, dag);
	builder.createDag();
	//cout << dag << endl;

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl;

	XmlWriter<TopTree<int>, int> writer(topTree);
	writer.write("/tmp/toptree.xml");

	TopTree<int> newTopTree(t._numNodes);
	BinaryDagUnpacker<int> dagUnpacker(dag, newTopTree);
	dagUnpacker.unpack();
	//cout << newTopTree << endl;

	OrderedTree<TreeNode, TreeEdge> newTree;
	Labels<int> newLabels;
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, int> unpacker(topTree, newTree, newLabels);
	unpacker.unpack();

	XmlWriter<OrderedTree<TreeNode, TreeEdge>, int> unpackedWriter(newTree, newLabels);
	unpackedWriter.write("/tmp/unpacked.xml");

//*/
	return 0;
}
