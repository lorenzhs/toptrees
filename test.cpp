#include <iostream>
#include <vector>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "TopTree.h"
#include "XML.h"
#include "Timer.h"


using std::cout;
using std::endl;

int main(int argc, char** argv) {
	OrderedTree<TreeNode,TreeEdge> t;
//
	std::vector<std::string> labels;

	std::string filename = argc > 1 ? std::string(argv[1]) : "data/1998statistics.xml";

	XmlParser<OrderedTree<TreeNode,TreeEdge>> xml(filename, t, labels);
	xml.parse();

	cout << t.summary() << endl;

	TopTree topTree(t._numNodes);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}

	Timer timer;
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});

	cout << "Top tree construction took " << timer.elapsedMillis() << "ms; Top tree has " << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;
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

	t.toString();

	TopTree topTree(t._numNodes);
	std::map<int,int> nodeIds;
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		cout << "merged " << u  << " and " << v << " into " << n << " type " << type << endl;
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});
	cout << endl << t << endl;
	cout << topTree << endl;
//*/
	return 0;
}
