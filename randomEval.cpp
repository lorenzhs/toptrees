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
#include "XML.h"

using std::cout;
using std::endl;

void usage(char* name) {
	cout << "Usage: " << name << " <options>" << endl
		 << "  -m <int>  tree size (edges) (default: 1000)" << endl
		 << "  -n <int>  number of trees to test (default: 100)" << endl
		 << "  -l <int>  number of different labels to assign to the nodes (default: 2)" << endl
		 << "  -s <int>  seed (default: 12345678)" << endl
		 << "  -r <file> set output file for edge compression ratios (default: no output)" << endl
		 << "  -o <file> set output file for debug information (default: no output)" << endl
		 << "  -w <path> set output folder for generated trees as XML files (default: don't write)" << endl
		 << "  -t <int>  number of threads to use (default: #cores)" << endl
		 << "  -v        verbose" << endl
		 << "  -vv       extra verbose" << endl;
}

std::mutex debugMutex;

void runIteration(const int iteration, RandomGeneratorType &generator, const uint seed, const int size, const int numLabels, const bool verbose, const bool extraVerbose, Statistics &statistics, ProgressBar &bar, const string &treePath) {
	// Seed RNG
	generator.seed(seed);
	if (verbose) cout << endl << "Round " << iteration << ", seed is " <<seed << endl;

	DebugInfo debugInfo;
	OrderedTree<TreeNode, TreeEdge> tree;
	RandomTreeGenerator<RandomGeneratorType> rand(generator);

	Timer timer;

	// Generate random tree
	rand.generateTree(tree, size);
	RandomLabels<RandomGeneratorType> labels(size + 1, numLabels, generator);

	debugInfo.generationDuration = timer.get();
	if (verbose) cout << "Generated " << tree.summary() << " in " << timer.get() << "ms" << endl;
	timer.reset();

	debugInfo.height = tree.height();
	debugInfo.avgDepth = tree.avgDepth();
	debugInfo.statDuration = timer.getAndReset();

	if (treePath != "") {
		const string filename(treePath + "/" + std::to_string(iteration) + "_" + std::to_string(seed) + ".xml");
		XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(tree, labels, filename);
		debugInfo.ioDuration = timer.getAndReset();
	}

	TopTree<int> topTree(tree._numNodes, labels);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);
	topTreeConstructor.construct(&debugInfo, verbose, extraVerbose);

	if (debugInfo.minEdgeRatio < 1.2) {
		cout << "minRatio = " << debugInfo.minEdgeRatio << " for seed " << seed << endl;
	}

	debugInfo.mergeDuration = timer.get();
	if (verbose)
		cout << "Top tree construction took " << timer.get() << "ms" << endl;
	timer.reset();

	debugInfo.topTreeHeight = topTree.height();
	debugInfo.statDuration += timer.getAndReset();

	BinaryDag<int> dag;
	DagBuilder<int> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	debugInfo.dagDuration = timer.get();
	if (verbose)
		cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
			 << "% of original tree, " << ratio << ":1)" << endl << "Top dag construction took "
			 << timer.get() << "ms" << endl;

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
	const string ratioFilename = argParser.get<string>("r", "");
	const string debugFilename = argParser.get<string>("o", "");
	const string treePath = argParser.get<string>("w", "");

	if (treePath != "") {
		makePathRecursive(treePath);
	}

	int numWorkers(std::thread::hardware_concurrency());
	numWorkers = argParser.get<int>("t", numWorkers);

	Statistics statistics(ratioFilename, debugFilename);
	ProgressBar bar(numIterations, std::cerr);

	cout << "Running experiments with " << numIterations << " trees of size " << size << " with " << numLabels
		 << " different labels" << flush;

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
			runIteration(i, engine, seeds[i], size, numLabels, verbose, extraVerbose, statistics, bar, treePath);
		}
	};

	const int treesPerThread = numIterations / numWorkers;
	const int extraForFirst = numIterations % numWorkers;

	vector<std::thread> workers;

	cout << " using " << numWorkers << " threads" << endl;

	for (int i = 0; i < numWorkers; ++i) {
		int min = i * treesPerThread;
		int max = min + treesPerThread;
		if (i == 0) max += extraForFirst;
		workers.push_back(std::thread(worker, min, max));
	}

	for (std::thread &worker : workers) {
		worker.join();
	}

	bar.undraw();

	statistics.compute();
	statistics.dump(std::cerr);
}
