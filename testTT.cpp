#include <iostream>
#include <vector>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "TopTree.h"
#include "XML.h"
#include "Timer.h"

#include "DagBuilder.h"
#include "TopTreeConstructor.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;

	Labels<string> labels(0);

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";
	string outputfolder = argc > 2 ? string(argv[2]) : "/tmp";

	// Read input file
	XmlParser<OrderedTree<TreeNode, TreeEdge>> xml(filename, t, labels);
	xml.parse();

	// Dump input file for comparison of output
	Timer timer;
	XmlWriter<OrderedTree<TreeNode, TreeEdge>> origWriter(t, labels);
	origWriter.write(outputfolder + "/orig.xml");

	cout << "Wrote orginial trimmed XML file in " << timer.getAndReset() << "ms: " << t.summary() << endl;

	// Prepare for construction of top tree
	TopTree<string> topTree(t._numNodes, labels);
	TopTreeConstructor<OrderedTree<TreeNode, TreeEdge>, string> topTreeConstructor(t, topTree);
	timer.reset();

	// construct top tree
	topTreeConstructor.construct();

	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size()
		 << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	// Construct top DAG
	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	cout << "Top DAG has " << dag.nodes.size() - 1 << " nodes, " << dag.countEdges() << " edges" << endl;
	cout << "Top DAG construction took in " << timer.getAndReset() << "ms" << endl;

	// Unpack top DAG to recoveredTopTree
	TopTree<string> recoveredTopTree(t._numNodes);
	BinaryDagUnpacker<string> dagUnpacker(dag, recoveredTopTree);
	dagUnpacker.unpack();

	cout << "Unpacked Top DAG in " << timer.getAndReset() << "ms, has " << recoveredTopTree.clusters.size()
		 << " clusters" << endl
		 << "Equality check... " << topTree.isEqual(recoveredTopTree) << endl;

	// unpack recovered top tree
	OrderedTree<TreeNode, TreeEdge> recoveredTree;
	Labels<string> newLabels(labels.numKeys());
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>, string> unpacker(recoveredTopTree, recoveredTree, newLabels);
	unpacker.unpack();
	cout << "Unpacked recovered top tree in " << timer.getAndReset() << "ms: " << recoveredTree.summary() << endl;

	XmlWriter<OrderedTree<TreeNode, TreeEdge>> unpackedWriter(recoveredTree, newLabels);
	unpackedWriter.write(outputfolder + "/unpacked.xml");
	cout << "Wrote recovered tree in " << timer.getAndReset() << "ms" << endl;

	return 0;
}
