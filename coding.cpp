#include <iostream>
#include <string>

// Data Structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "TopTree.h"

// Algorithms
#include "TopDagUnpacker.h"
#include "Entropy.h"
#include "RePairCombiner.h"
#include "TopDagConstructor.h"
#include "TreeSizeEstimation.h"

// Utils
#include "ArgParser.h"
#include "FileWriter.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	ArgParser argParser(argc, argv);
	const bool useRePair = argParser.isSet("r");
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	} else if (useRePair) {
		// if used as "./coding -r foo.xml", it will match the foo.xml to the "-r" which is unfortunate
		string arg = argParser.get<string>("r", "");
		filename = (arg == "") ? filename : arg;
	}
	const double minRatio = argParser.get<double>("m", 1.22);

	OrderedTree<TreeNode, TreeEdge> t;
	Labels<string> labels;

	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	const int origNodes(t._numNodes), origEdges(t._numEdges), origHeight(t.height());
	const double origAvgDepth(t.avgDepth());
	cout << t.summary() << "; Height: " << origHeight << " Avg depth: " << origAvgDepth << endl;

	TopDag<string> dag(t._numNodes, labels);
	const long long treeSize = TreeSizeEstimation<OrderedTree<TreeNode, TreeEdge>>::compute(t, labels);

	Timer timer;
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct(NULL, minRatio);
	} else {
		TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct();
	}
	cout << "Top DAG construction took " << timer.getAndReset() << "ms" << endl;
/*
	const double ttAvgDepth(topTree.avgDepth());
	const int ttMinDepth(topTree.minDepth()), ttHeight(topTree.height());
	cout << "avg node depth " << ttAvgDepth << " (min " << ttMinDepth << ", height " << ttHeight << "); "
	     << "took " << timer.getAndReset() << "ms" << endl;
*/

	const int edges(dag.countEdges()), nodes((int)dag.nodes.size() - 1);
	const double edgePercentage = (edges * 100.0) / origEdges;
	const double nodePercentage = (nodes * 100.0) / origNodes;
	const double ratio = ((int)(1000 / edgePercentage)) / 10.0;
	cout << "Top dag has " << nodes << " nodes (" << nodePercentage << "%), "
		 << edges << " edges (" << edgePercentage << "% of original tree, " << ratio << ":1)" << endl;

	long long bits = FileWriter::write(dag, labels, "/tmp/foo");

	const std::streamsize precision = cout.precision();
	cout << "Output file needs " << bits << " bits (" << (bits+7)/8 << " bytes), vs " << (treeSize+7)/8 << " bytes for orig succ tree, "
		 << std::fixed << std::setprecision(1) << (double)treeSize/bits << ":1" << endl;
	cout.unsetf(std::ios_base::fixed);
	cout << std::setprecision(precision);

	cout << "RESULT"
		 << " compressed=" << bits
		 << " succinct=" << treeSize
		 << " minRatio=" << minRatio
		 << " repair=" << useRePair
		 << " nodes=" << nodes
		 << " origNodes=" << origNodes
		 << " edges=" << edges
		 << " origEdges=" << origEdges
		 << " file=" << filename
		 << " origHeight=" << origHeight
		 << " origAvgDepth=" << origAvgDepth
		 //<< " ttAvgDepth=" << ttAvgDepth
		 //<< " ttMinDepth=" << ttMinDepth
		 //<< " ttHeight=" << ttHeight
		 << endl;

	return 0;
}
