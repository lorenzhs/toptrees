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

#include "ArgParser.h"

using std::cout;
using std::endl;

void usage(char* name) {
	cout << "Usage: " << name << " <options>" << endl
		 << "  -m <int>  tree size (nodes) (default: 1000)" << endl
		 << "  -n <int>  number of trees to test (default: 100)" << endl
		 << "  -l <int>  number of different labels to assign to the nodes (default: 2)" << endl
		 << "  -s <int>  seed (default: 12345678)" << endl
		 << "  -o <file> set output file for edge compression ratios (empty for no output)" << endl
		 << "  -v        verbose" << endl
		 << "  -vv       extra verbose" << endl;
}

int main(int argc, char **argv) {
	ArgParser argParser(argc, argv);

	if (argParser.isSet("h") || argParser.isSet("-help")) {
		usage(argv[0]);
		return 0;
	}

	const int size = argParser.get<int>("m", 1000);
	const int numIterations = argParser.get<int>("n", 100);
	const uint numLabels = argParser.get<uint>("l", 2);
	const uint seed = argParser.get<uint>("s", 12345678);
	const bool verbose = argParser.isSet("v") || argParser.isSet("vv");
	const bool extraVerbose = argParser.isSet("vv");
	const string filename = argParser.get<string>("o", "ratios.dat");

	Timer timer;
	Statistics statistics;
	ProgressBar bar(numIterations, std::cerr);

	cout << "Running experiments with " << numIterations << " trees of size " << size << " with " << numLabels
		 << " different labels" << endl;

	// Generate seeds deterministically from the input parameters
	vector<uint> seeds(numIterations);
	if (numIterations > 1) {
		std::seed_seq seedSeq({(uint)size, (uint)numIterations, numLabels, seed});
		seedSeq.generate(seeds.begin(), seeds.end());
	} else {
		// Allow reconstructing a single tree
		seeds[0] = seed;
	}

	for (int iteration = 0; iteration < numIterations; ++iteration) {
		// Seed RNG
		getRandomGenerator().seed(seeds[iteration]);
		if (verbose) cout << endl << "Round " << iteration << ", seed is " << seeds[iteration] << endl;

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
		debugInfo.avgDepth = tree.avgDepth();
		timer.reset();

		TopTree<int> topTree(tree._numNodes, labels);
		TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);
		topTreeConstructor.construct(&debugInfo, verbose, extraVerbose);

		auto minRatio = std::min_element(debugInfo.edgeRatios.begin(), debugInfo.edgeRatios.end());
		if (minRatio != debugInfo.edgeRatios.end() && *minRatio < 1.2) {
			cout << "minRatio = " << *minRatio << " for seed " << seeds[iteration] << endl;
		}

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
	statistics.dump(std::cerr);
	if (filename != "")
		statistics.dumpEdgeRatioDistribution(filename);
}
