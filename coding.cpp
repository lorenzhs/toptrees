#include <iostream>
#include <string>

// Data Structures
#include "BinaryDag.h"
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopTree.h"

// Algorithms
#include "DagBuilder.h"
#include "Entropy.h"
#include "RePairCombiner.h"
#include "TopTreeConstructor.h"

// Utils
#include "ArgParser.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	ArgParser argParser(argc, argv);
	const bool useRePair = argParser.isSet("r");
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	} else if (useRePair) {
		// if used as "./coding -r foo.xml", it will match the foo.xml to the "-r" which is unfortunate
		string arg = argParser.get<string>("r", "");
		filename = (arg == "") ? filename : arg;
	}

	OrderedTree<TreeNode, TreeEdge> t;
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
		 << "Top dag construction took in " << timer.getAndReset() << "ms" << endl;

	DagEntropy<string> entropy(dag, labels);
	entropy.calculate();
	cout << "DAG Structure: " << entropy.dagStructureEntropy.huffman << endl;
	cout << "DAG Pointers:  " << entropy.dagPointerEntropy << endl;
	cout << "Merge Types:   " << entropy.mergeEntropy.huffman << endl;
	cout << "Label strings: " << entropy.labelDataEntropy.huffman << " + " << entropy.labelDataEntropy.getExtraSize() << " bits for symbols" << endl;
	cout << "Huffman calcuation took " << timer.getAndReset() << "ms; " << endl;
	long long bits = entropy.getTotalSize();
	cout << "Output file needs " << bits << " bits (" << (bits+7)/8 << " bytes)" << endl;
	return 0;
}
