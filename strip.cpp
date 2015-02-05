#include <iostream>
#include <string>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "XML.h"

#include "ArgParser.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	OrderedTree<TreeNode, TreeEdge> t;

	Labels<string> labels(0);

	ArgParser argParser(argc, argv);
	string filename = argParser.get<string>("i", "data/1998statistics.xml");
	string outputfolder = argParser.get<string>("o", "/tmp");

	// Read input file
	XmlParser<OrderedTree<TreeNode, TreeEdge>>::parse(filename, t, labels);

	const int nodes(t._numNodes), height(t.height());
	const double avgDepth(t.avgDepth());
	cout << t.summary() << "; Height: " << height << " Avg depth: " << avgDepth << endl;

	// Dump input file for comparison of output
	Timer timer;
	auto pos = filename.find_last_of("/\\");
	string outname = outputfolder + "/" + filename.substr(pos + 1) + ".stripped";
	XmlWriter<OrderedTree<TreeNode, TreeEdge>>::write(t, labels, outname, false);

	cout << "Wrote trimmed XML file in " << timer.getAndReset() << "ms: " << t.summary() << endl;

	// Get size
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    auto origSize = in.tellg();
	std::ifstream in2(outname, std::ifstream::ate | std::ifstream::binary);
    auto strippedSize = in2.tellg();

    cout << "RESULT"
    	 << " file=" << filename
    	 << " origSize=" << origSize
    	 << " strippedSize=" << strippedSize
    	 << " nodes=" << nodes
    	 << " height=" << height
    	 << " avgDepth=" << avgDepth
    	 << endl;

	return 0;
}
