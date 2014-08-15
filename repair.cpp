#include <iostream>
#include <string>

// Data Structures
#include "RePair/RePair.h"
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

template <typename InType, typename DataType>
void compress(vector<InType> &data, const std::string &description, const bool verbose = false) {
	Timer timer;
	cout << "RePair-ing the " << description << ", initialising… " << flush;
	std::vector<DataType> compressedStructure;
	RePair::RePair<DataType, InType> structureRePair(data);
	cout << timer.getAndReset() << "ms, compressing… " << flush;
	structureRePair.compress(compressedStructure);
	cout << "done (" << timer.getAndReset() << "ms)" << endl;

	if (verbose) {
		for (auto elem : compressedStructure) {
			std::cout << elem << " ";
		}
		std::cout << std::endl << structureRePair.getDictionary();
	}

}

int main(int argc, char **argv) {
	ArgParser argParser(argc, argv);
	string filename = "data/1998statistics.xml";
	if (argParser.numDataArgs() > 0) {
		filename = argParser.getDataArg(0);
	}
	const bool verbose = argParser.isSet("v");

	OrderedTree<TreeNode, TreeEdge> tree;
	Labels<string> labels;
	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, tree, labels);
	cout << tree.summary() << "; Height: " << tree.height() << " Avg depth: " << tree.avgDepth() << endl;

	Timer timer;
	std::vector<unsigned char> labelnames;
	std::vector<bool> bpstring;
	BPString::template fromTree<TreeNode, TreeEdge, string>(tree, labels, bpstring, labelnames);

	cout << "bpstring with " << bpstring.size() << " bits, " << labelnames.size() << " bytes of labels (transformation took " << timer.getAndReset() << "ms)" << endl;

	compress<bool, int>(bpstring, "tree structure", verbose);
	compress<unsigned char, int>(labelnames, "labels", verbose);
	return 0;
}
