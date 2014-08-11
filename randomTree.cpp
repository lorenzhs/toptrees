#include <iostream>

// Data Structures
#include "BinaryDag.h"
#include "Edges.h"
#include "Labels.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopTree.h"

// Algorithms
#include "DagBuilder.h"
#include "DotGraphExporter.h"
#include "RandomTree.h"
#include "TopTreeConstructor.h"

// Utils
#include "Timer.h"

using std::cout;
using std::endl;

int main(int argc, char **argv) {
	int size = argc > 1 ? std::stoi(argv[1]) : 10;
	int seed = argc > 2 ? std::stoi(argv[2]) : 12345678;
	getRandomGenerator().seed(seed);
	RandomTreeGenerator<RandomGeneratorType> rand(getRandomGenerator());
	OrderedTree<TreeNode, TreeEdge> tree;

	Timer timer;
	rand.generateTree(tree, size, (size < 1000));
	cout << "Generated " << tree.summary() << " in " << timer.getAndReset() << "ms" << endl;

	if (size <= 30) cout << tree << endl;

	if (size <= 10000) {
		OrderedTreeDotGraphExporter<TreeNode, TreeEdge>().write(tree, "/tmp/tree.dot");
		cout << "Wrote DOT file in " << timer.getAndReset() << "ms" << endl;
	}

	if (size <= 1000) {
		DotGraphExporter<OrderedTree<TreeNode, TreeEdge>>::drawSvg("/tmp/tree.dot", "/tmp/tree.svg");
		cout << "Graphed DOT file in " << timer.getAndReset() << "ms" << endl;
	}

	IdLabels labels(10);
	TopTree<int> topTree(tree._numNodes, labels);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);

	timer.reset();
	topTreeConstructor.construct();
	cout << "Top tree construction took " << timer.getAndReset() << "ms" << endl;

	BinaryDag<int> dag;
	DagBuilder<int> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl
		 << "Top dag construction took in " << timer.get() << "ms" << endl;
}
