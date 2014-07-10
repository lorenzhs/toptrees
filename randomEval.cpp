#include <iostream>
#include <string>

#include "Common.h"
#include "RandomTree.h"

#include "OrderedTree.h"
#include "Nodes.h"
#include "Edges.h"

#include "Timer.h"

#include "DotGraphExporter.h"

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
	uint numLabels = argc > 3 ? std::stoi(argv[3]) : 1;
	uint seed = argc > 4 ? std::stoi(argv[4]) : 12345678;
	const bool verbose = false;

	IdLabels labels(numLabels);
	RandomTreeGenerator rand;

	Timer timer;
	Statistics statistics;
	ProgressBar bar(numIterations);

	cout << "Running experiments with " << numIterations << " trees of size " << size << " with " << numLabels << " different labels" << endl;

	for (int iteration = 0; iteration < numIterations; ++iteration) {
		DebugInfo debugInfo;
		boost_hash_combine(seed, iteration);
		rand.seed(seed);

		OrderedTree<TreeNode, TreeEdge> tree;
		timer.reset();
		rand.generateTree(tree, size);
		debugInfo.generationDuration = timer.elapsedMillis();
		if (verbose) cout << "Generated " << tree.summary() << " in " << timer.getAndReset() << "ms" << endl;
		debugInfo.height = tree.height();
		timer.reset();

		vector<int> nodeIds(size);
		for (int i = 0; i < size; ++i) {
			nodeIds[i] = i;
		}

		TopTree<int> topTree(tree._numNodes, labels);

		tree.doMerges([&](const int u, const int v, const int n,
						  const MergeType type) { nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type); },
					  verbose);
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
