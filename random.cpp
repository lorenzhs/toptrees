#include <iostream>
#include <string>

#include "RandomTree.h"

#include "OrderedTree.h"
#include "Nodes.h"
#include "Edges.h"

#include "Timer.h"

#include "DotGraphExporter.h"

#include "TopTree.h"
#include "Labels.h"
#include "TopTreeConstructor.h"

#include "BinaryDag.h"
#include "DagBuilder.h"

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
		DotGraphExporter::template write<OrderedTree<TreeNode, TreeEdge>>(tree, "/tmp/tree.dot");
		cout << "Wrote DOT file in " << timer.getAndReset() << "ms" << endl;
	}

	if (size <= 1000) {
		DotGraphExporter::drawSvg("/tmp/tree.dot", "/tmp/tree.svg");
		cout << "Graphed DOT file in " << timer.getAndReset() << "ms" << endl;
	}

	IdLabels labels(10);
	TopTree<int> topTree(tree._numNodes, labels);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);

	timer.reset();
	topTreeConstructor.construct();
	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size()
		 << " clusters (" << topTree.clusters.size() - tree._numNodes << " non-leaves)" << endl;

	BinaryDag<int> dag;
	DagBuilder<int> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl
		 << "Top dag construction took in " << timer.elapsedMillis() << "ms" << endl;
}
