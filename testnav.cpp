/*
 * Preorder traversal of an in-memory Top DAG.
 *
 * Not particularly useful except to test whether
 * navigation works.
 */

#include <algorithm>
#include <iostream>
#include <string>

// Data structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "TopTree.h"

// Algorithms
#include "TopDagConstructor.h"
#include "TopTreeUnpacker.h"
#include "RePairCombiner.h"
#include "TopDagUnpacker.h"
#include "Navigation.h"
#include "NavTest.h"

// Utils
#include "ArgParser.h"
#include "DotGraphExporter.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;
	ArgParser argParser(argc, argv);
	const bool useRePair = argParser.isSet("r");
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	} else if (useRePair) {
		// if used as "./test -r foo.xml", it will match the foo.xml to the "-r" which is unfortunate
		string arg = argParser.get<string>("r", "");
		filename = (arg == "") ? filename : arg;
	}
	const bool print = argParser.isSet("p");

	Labels<string> labels;
	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);
	cout << t.summary() << "; Height: " << t.height() << " Avg depth: " << t.avgDepth() << endl;

	const int treeEdges = t._numEdges;
	TopDag<string> dag(t._numNodes, labels);

	Timer timer;
	if (useRePair) {
		RePairCombiner<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct();
	} else {
		TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, string> topDagConstructor(t, dag);
		topDagConstructor.construct();
	}
	cout << "Top DAG construction took " << timer.getAndReset() << "ms";
	//, avg node depth " << topTree.avgDepth() << " (min " << topTree.minDepth() << "); took " << timer.getAndReset() << " ms" << endl;

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / treeEdges;
	const double ratio = ((int)(1000 / percentage)) / 10.0;
	cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
		 << "% of original tree, " << ratio << ":1)" << endl;

	/*Navigator<string> nav(dag);

	cout << "isLeaf: " << nav.isLeaf() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "firstChild: " << nav.firstChild() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "firstChild: " << nav.firstChild() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "firstChild: " << nav.firstChild() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "nextSibling: " << nav.nextSibling() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "nextSibling: " << nav.nextSibling() << "; label: " << nav.getLabel() << " = " << std::endl << std::endl;

	cout << "parent: " << nav.parent() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "nextSibling: " << nav.nextSibling() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "firstChild: " << nav.firstChild() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "parent: " << nav.parent() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
	cout << "parent: " << nav.parent() << "; label: " << nav.getLabel() << " = " << std::flush << *nav.getLabel() << std::endl << std::endl;
*/

	PreorderTraversal<string> trav(dag, print);
        unsigned long long nodesVisited, maxTreeStackSize;
        timer.reset();
        std::tie(nodesVisited, maxTreeStackSize) = trav.run();
        cout << "Preorder    traversal took " << timer.get() << "ms, visited "
             << nodesVisited << " nodes; max tree stack size = "
             << maxTreeStackSize << " Bytes" << endl;

        timer.reset();
        std::tie(nodesVisited, maxTreeStackSize) = trav.runRight();
        cout << "Right-first traversal took " << timer.get() << "ms, visited "
             << nodesVisited << " nodes; max tree stack size = "
             << maxTreeStackSize << " Bytes" << endl;

	return 0;
}
