/*
 * Test correctness top tree compression with random trees
 *
 * Generates lots of trees uniformly at random, compresses
 * them, then decompresses that and checks whether the result
 * matches the original tree. Supports both classic TTC and
 * the RePair combiner.
 */

#include <iostream>
#include <string>

#include <thread>
#include <mutex>

#include "Common.h"

// Data Structures
#include "Edges.h"
#include "Labels.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "TopTree.h"

// Algorithms
#include "RandomTree.h"
#include "TopDagUnpacker.h"
#include "RePairCombiner.h"
#include "TopDagConstructor.h"
#include "TopTreeUnpacker.h"

// Utils
#include "ArgParser.h"
#include "ProgressBar.h"
#include "Statistics.h"
#include "Timer.h"
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

void runIteration(const int iteration, RandomGeneratorType &generator, const uint seed, const int size,
		const int numLabels, const bool useRePair, const bool verbose, const bool extraVerbose,
		Statistics &statistics, ProgressBar &bar, const string &treePath) {
	// Seed RNG
	generator.seed(seed);
	if (verbose) cout << endl << "Round " << iteration << ", seed is " <<seed << endl;

	DebugInfo debugInfo;
	OrderedTree<TreeNode, TreeEdge> tree;
	RandomTreeGenerator<RandomGeneratorType> rand(generator);

	Timer timer;

	// Generate random tree
	rand.generateTree(tree, size);
	RandomLabels<RandomGeneratorType> labels(size+1, numLabels, generator);

	debugInfo.generationDuration = timer.get();
	if (verbose) cout << "Generated " << tree.summary() << " in " << timer.get() << "ms" << endl;
	timer.reset();

	if (treePath != "") {
		const string filename(treePath + "/" + std::to_string(iteration) + "_orig.xml");
		XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(tree, labels, filename);
		debugInfo.ioDuration = timer.get();
		if (verbose) cout << "Wrote orginial trimmed XML file in " << timer.get() << "ms: " << tree.summary() << endl;
		timer.reset();
	}

	OrderedTree<TreeNode, TreeEdge> treeCopy(tree);
	debugInfo.height = tree.height();
	debugInfo.avgDepth = tree.avgDepth();
	debugInfo.statDuration = timer.getAndReset();

	// Construct top tree
	TopDag<int> dag(tree._numNodes, labels);
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, int> topDagConstructor(tree, dag, verbose, extraVerbose);
		topDagConstructor.construct(&debugInfo);
	} else {
		TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, int> topDagConstructor(tree, dag, verbose, extraVerbose);
		topDagConstructor.construct(&debugInfo);
	}

	debugInfo.mergeDuration = timer.get();
	if (verbose)
		cout << "Top DAG construction took " << timer.get() << "ms" << endl;
	timer.reset();

/*
	debugInfo.topTreeHeight = topTree.height();
	debugInfo.topTreeAvgDepth = topTree.avgDepth();
	debugInfo.topTreeMinDepth = topTree.minDepth();
	debugInfo.statDuration += timer.getAndReset();
*/

	// Unpack top DAG to topTree
	TopTree<int> topTree(size + 1);
	TopDagUnpacker<int> dagUnpacker(dag, topTree);
	dagUnpacker.unpack();

	// Unpack top tree
	OrderedTree<TreeNode, TreeEdge> unpackedTree;
	Labels<int> newLabels(size + 1);
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, int> treeUnpacker(topTree, unpackedTree, newLabels);
	treeUnpacker.unpack();
	debugInfo.unpackDuration = timer.get();
	if (verbose) cout << "Unpacked top tree in " << timer.get() << "ms" << flush;
	timer.reset();

	// Verify that the unpacked tree is identical to the original tree
	if (!unpackedTree.isEqual<LabelsT<int>>(treeCopy, newLabels, labels)) {
		std::cerr << "Top Tree unpacking produced incorrect result for seed " << seed << endl;
	}
	debugInfo.statDuration += timer.get();
	if (verbose) cout << "; checked in " << timer.get() << "ms" << endl;
	timer.reset();


	if (treePath != "") {
		const string filename(treePath + "/" + std::to_string(iteration) + "_unpacked.xml");
		XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(unpackedTree, newLabels, filename);
		debugInfo.ioDuration += timer.get();
		if (verbose) cout << "Wrote recovered tree in " << timer.get() << "ms" << endl;
	}

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
	const bool useRePair = argParser.isSet("r");
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

	Timer timer;
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
			runIteration(i, engine, seeds[i], size, numLabels, useRePair, verbose, extraVerbose, statistics, bar, treePath);
		}
	};

	const int treesPerThread = numIterations / numWorkers;
	const int leftovers = numIterations % numWorkers;

	vector<std::thread> workers;

	cout << " using " << numWorkers << " threads" << endl;

	int min(0), max(treesPerThread);
	for (int i = 0; i < numWorkers; ++i) {
		if (i < leftovers) {
			max++;
		}
		workers.push_back(std::thread(worker, min, max));
		min = max;
		max += treesPerThread;
	}

	for (std::thread &worker : workers) {
		worker.join();
	}

	bar.undraw();

	statistics.compute();
	statistics.dump(std::cerr);
}
