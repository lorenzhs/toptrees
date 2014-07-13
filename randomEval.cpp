#include <iostream>
#include <string>

#include "Common.h"
#include "RandomTree.h"

#include "OrderedTree.h"
#include "Nodes.h"
#include "Edges.h"

#include "Timer.h"

#include "TopTreeConstructor.h"

#include "TopTree.h"
#include "Labels.h"

#include "BinaryDag.h"
#include "DagBuilder.h"

#include "Statistics.h"
#include "ProgressBar.h"

using std::cout;
using std::endl;

int main(int argc, char **argv) {
	int size = argc > 1 ? std::stoi(argv[1]) : 1000;
	int numIterations = argc > 2 ? std::stoi(argv[2]) : 100;
	uint numLabels = argc > 3 ? std::stoi(argv[3]) : 2;
	uint seed = argc > 4 ? std::stoi(argv[4]) : 12345678;
	const bool verbose = false;

	Timer timer;
	Statistics statistics;
	ProgressBar bar(numIterations);

	cout << "Running experiments with " << numIterations << " trees of size " << size << " with " << numLabels
		 << " different labels" << endl;

	// Generate seeds deterministically from the input parameters
	std::seed_seq seedSeq({(uint)size, (uint)numIterations, numLabels, seed});
	vector<uint> seeds(numIterations);
	seedSeq.generate(seeds.begin(), seeds.end());

	for (int iteration = 0; iteration < numIterations; ++iteration) {
		// Seed RNG
		getRandomGenerator().seed(seeds[iteration]);

		DebugInfo debugInfo;
		OrderedTree<TreeNode, TreeEdge> tree;
		RandomTreeGenerator<RandomGeneratorType> rand(getRandomGenerator());
		timer.reset();

		// Generate random tree
		rand.generateTree(tree, size);
		RandomLabels<RandomGeneratorType> labels(size, numLabels, getRandomGenerator());

		debugInfo.generationDuration = timer.elapsedMillis();
		if (verbose) cout << "Generated " << tree.summary() << " in " << timer.getAndReset() << "ms" << endl;
		debugInfo.height = tree.height();
		timer.reset();

		TopTree<int> topTree(tree._numNodes, labels);
		TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);
		topTreeConstructor.construct(&debugInfo, verbose);

		debugInfo.mergeDuration = timer.elapsedMillis();
		if (verbose)
			cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has "
				 << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - tree._numNodes
				 << " non-leaves)" << endl;

		BinaryDag<int> dag;
		DagBuilder<int> builder(topTree, dag);
		builder.createDag();

		const int edges = dag.countEdges();
		const double percentage = (edges * 100.0) / topTree.numLeaves;
		const double ratio = ((int)(1000 / percentage)) / 10.0;
		debugInfo.dagDuration = timer.elapsedMillis();
		if (verbose)
			cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
				 << "% of original tree, " << ratio << ":1)" << endl << "Top dag construction took in "
				 << timer.elapsedMillis() << "ms" << endl;

		debugInfo.numDagEdges = edges;
		debugInfo.numDagNodes = dag.nodes.size() - 1;
		statistics.addDebugInfo(debugInfo);

		++bar;
	}
	bar.undraw();

	statistics.compute();
	statistics.dump(cout);
}
