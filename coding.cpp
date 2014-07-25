#include <iostream>
#include <vector>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "TopTree.h"
#include "TopTreeConstructor.h"
#include "XML.h"
#include "Timer.h"

#include "DagBuilder.h"

#include "Entropy.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;
	Labels<string> labels;

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";

	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	cout << t.summary() << "; Height: " << t.height() << " Avg depth: " << t.avgDepth() << endl;

	TopTree<string> topTree(t._numNodes, labels);

	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, string> topTreeConstructor(t, topTree);
	Timer timer;
	topTreeConstructor.construct();
	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size()
		 << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl
		 << "Top dag construction took in " << timer.getAndReset() << "ms" << endl;

	DagEntropy<string> entropy(dag);
	entropy.calculate();
	cout << "DAG: " << entropy.getDagEntropy().summary();
	cout << "Labels: " << entropy.getLabelEntropy().summary();
	cout << "MergeT: " << entropy.getMergeEntropy().summary();
	double dagEntropyTime = timer.getAndReset();

	StringLabelEntropy labelEntropy(labels);
	labelEntropy.calculate();
	cout << "Label strings: " << labelEntropy.getEntropy().summary();

	double labelEntropyTime = timer.getAndReset();
	cout << "Entropy calcuation took " << dagEntropyTime << " + " << labelEntropyTime << " = " << dagEntropyTime + labelEntropyTime << "ms; " << endl;

	int bits = entropy.getDagEntropy().huffBitsNeeded() +
		entropy.getLabelEntropy().huffBitsNeeded() +
		entropy.getMergeEntropy().huffBitsNeeded() +
		labelEntropy.getEntropy().huffBitsNeeded();
	cout << "Output needs " << bits << " bits (" << (bits+7)/8 << " bytes)" << endl;
	return 0;
}
