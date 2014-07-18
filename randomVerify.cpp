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
	RandomLabels<RandomGeneratorType> labels(size+1, numLabels, generator);

	debugInfo.generationDuration = timer.elapsedMillis();
	if (verbose) cout << "Generated " << tree.summary() << " in " << timer.getAndReset() << "ms" << endl;

	if (treePath != "") {
		const string filename(treePath + "/" + std::to_string(iteration) + "_orig.xml");
		XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(tree, labels, filename);
		if (verbose) cout << "Wrote orginial trimmed XML file in " << timer.getAndReset() << "ms: " << tree.summary() << endl;
	}

	OrderedTree<TreeNode, TreeEdge> treeCopy(tree);
	debugInfo.height = tree.height();
	debugInfo.avgDepth = tree.avgDepth();

	timer.reset();

	// Construct top tree
	TopTree<int> topTree(tree._numNodes, labels);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, int> topTreeConstructor(tree, topTree);
	topTreeConstructor.construct(&debugInfo, verbose, extraVerbose);

	debugInfo.mergeDuration = timer.elapsedMillis();
	if (verbose)
		cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has "
			 << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - tree._numNodes
			 << " non-leaves)" << endl;

	// Unpack top tree
	OrderedTree<TreeNode, TreeEdge> unpackedTree;
	Labels<int> newLabels(size + 1);
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, int> treeUnpacker(topTree, unpackedTree, newLabels);
	treeUnpacker.unpack();

	// Verify that the unpacked tree is identical to the original tree
	if (!unpackedTree.isEqual<LabelsT<int>>(treeCopy, newLabels, labels)) {
		std::cerr << "Top Tree unpacking produced incorrect result for seed " << seed << endl;
	}

	if (treePath != "") {
		const string filename(treePath + "/" + std::to_string(iteration) + "_unpacked.xml");
		XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(unpackedTree, newLabels, filename);
		if (verbose) cout << "Wrote recovered tree in " << timer.getAndReset() << "ms" << endl;
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
