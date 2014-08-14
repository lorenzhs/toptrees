#include <iostream>
#include <string>

// Data Structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"


// Utils
#include "ArgParser.h"
#include "BPString.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	ArgParser argParser(argc, argv);
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	}

	OrderedTree<TreeNode, TreeEdge> tree;
	Labels<string> labels;
	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, tree, labels);
	cout << tree.summary() << "; Height: " << tree.height() << " Avg depth: " << tree.avgDepth() << endl;

	std::vector<unsigned char> labelnames;
	std::vector<bool> bpstring;
	BPString::template fromTree<TreeNode, TreeEdge, string>(tree, labels, bpstring, labelnames);

	cout << "bpstring with " << bpstring.size() << " bits, " << labelnames.size() << " bytes of labels" << endl;

	return 0;
}
