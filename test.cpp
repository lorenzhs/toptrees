#include <iostream>
#include <vector>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "TopTree.h"
#include "XML.h"
#include "Timer.h"

#include "DagBuilder.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char** argv) {
	OrderedTree<TreeNode,TreeEdge> t;
//*
	vector<string> labels;

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";
	string outFilename = argc > 2 ? string(argv[2]) : "/tmp/toptree.xml";

	XmlParser<OrderedTree<TreeNode,TreeEdge>> xml(filename, t, labels);
	xml.parse();

	cout << t.summary() << endl;

	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}

	Timer timer;
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});

	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	XmlWriter writer(topTree);
	writer.write(outFilename);
	cout << "Wrote top tree to " << outFilename << " in " << timer.getAndReset() << "ms" << endl;

	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	cout << "Top dag has " << dag.nodes.size() - 1<< " nodes, " << dag.countEdges() << " edges" << endl;
	cout << "Top dag construction took in " << timer.elapsedMillis() << "ms" << endl;

/*/
	t.addNodes(11);
	t.addEdge(0, 1);
	t.addEdge(0, 2);
	t.addEdge(0, 3);
	t.addEdge(1, 4);
	t.addEdge(1, 5);
	t.addEdge(3, 6);
	t.addEdge(6, 7);
	t.addEdge(7, 8);
	t.addEdge(4, 9);
	t.addEdge(4, 10);
	cout << t << endl << endl;

	vector<string> labels({"root", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain"});

	t.toString();

	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		cout << "merged " << u  << " and " << v << " into " << n << " type " << type << endl;
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});
	cout << endl << t << endl;
	cout << topTree << endl;
	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();
	cout << dag << endl;

	XmlWriter writer(topTree);
	writer.write("/tmp/toptree.xml");

//*/
	return 0;
}
