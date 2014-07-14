#include <iostream>
#include <string>

#include <thread>
#include <mutex>

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

std::mutex debugMutex;

void runIteration(const int iteration, RandomGeneratorType &generator, const int seed, const int size, const int numLabels, const bool verbose, const bool extraVerbose, Statistics &statistics, ProgressBar &bar) {
	// Seed RNG
	generator.seed(seed);
	if (verbose) cout << endl << "Round " << iteration << ", seed is " <<seed << endl;

	DebugInfo debugInfo;
	OrderedTree<TreeNode, TreeEdge> tree;
	RandomTreeGenerator<RandomGeneratorType> rand(generator);

	Timer timer;

	// Generate random tree
	rand.generateTree(tree, size);
	RandomLabels<RandomGeneratorType> labels(size, numLabels, generator);

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
		cout << "minRatio = " << *minRatio << " for seed " << seed << endl;
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

	debugMutex.lock();
	statistics.addDebugInfo(debugInfo);
	++bar;
	debugMutex.unlock();
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

	// function that the threads will execute
	auto worker = [&](int start, int end) {
		RandomGeneratorType engine{};
		for (int i = start; i < end; ++i) {
			runIteration(i, engine, seeds[i], size, numLabels, verbose, extraVerbose, statistics, bar);
		}
	};

	const int numWorkers(std::thread::hardware_concurrency());
	const int treesPerThread = numIterations / numWorkers;
	const int extraForFirst = numIterations % numWorkers;

	vector<std::thread> workers;

	for (int i = 0; i < numWorkers; ++i) {
		int min = i * treesPerThread;
		int max = min + treesPerThread;
		if (i == 0) max += extraForFirst;
		cout << "Worker " << i << ": trees [" << min << ", " << max << ")" << endl;
		workers.push_back(std::thread(worker, min, max));
	}

	for (std::thread &worker : workers) {
		worker.join();
	}

	bar.undraw();

	statistics.compute();
	statistics.dump(std::cerr);
	if (filename != "")
		statistics.dumpEdgeRatioDistribution(filename);
}
