#include <iostream>
#include <string>

#include "RandomTree.h"

#include "OrderedTree.h"
#include "Nodes.h"
#include "Edges.h"

#include "Timer.h"

#include "DotGraphExporter.h"

using std::cout;
using std::endl;

int main(int argc, char** argv) {
	int size = argc > 1 ? std::stoi(argv[1]) : 10;
	int seed = argc > 2 ? std::stoi(argv[2]) : 12345678;
	RandomTreeGenerator rand;
	OrderedTree<TreeNode, TreeEdge> tree;

	Timer timer;
	rand.generateTree(tree, size, seed, (size < 1000));
	cout << "Generated " << tree.summary() << " in " << timer.getAndReset() << "ms" << endl;

	if (size <= 30)
		cout << tree << endl;

	if (size <= 10000) {
		DotGraphExporter::template write<OrderedTree<TreeNode, TreeEdge>>(tree, "/tmp/tree.dot");
		cout << "Wrote DOT file in " << timer.getAndReset() << "ms" << endl;
	}

	if (size <= 1000) {
		DotGraphExporter::drawSvg("/tmp/tree.dot", "/tmp/tree.svg");
		cout << "Graphed DOT file in " << timer.getAndReset() << "ms" << endl;
	}
}
