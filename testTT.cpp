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

	vector<string> labels;

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";

	XmlParser<OrderedTree<TreeNode,TreeEdge>> xml(filename, t, labels);
	xml.parse();

	Timer timer;
	XmlWriter<OrderedTree<TreeNode,TreeEdge>> origWriter(t, labels);
	origWriter.write("/tmp/orig.xml");

	cout << "Wrote orginial trimmed XML file in " << timer.getAndReset() << "ms: " << t.summary() << endl;

	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}

	timer.reset();
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});

	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	XmlWriter<TopTree> writer(topTree);
	writer.write("/tmp/toptree.xml");
	cout << "Wrote top tree in " << timer.getAndReset() << "ms" << endl;


	OrderedTree<TreeNode,TreeEdge> unpackedTree;
	TopTreeUnpacker<OrderedTree<TreeNode,TreeEdge>> unpacker(topTree, unpackedTree);
	unpacker.unpack();
	cout << "Unpacked top tree in " << timer.getAndReset() << "ms: " << unpackedTree.summary() << endl;
	XmlWriter<OrderedTree<TreeNode,TreeEdge>> unpackedWriter(unpackedTree, labels);
	unpackedWriter.write("/tmp/unpacked.xml");
	cout << "Wrote unpacked top tree in " << timer.getAndReset() << "ms" << endl;

	return 0;
}
