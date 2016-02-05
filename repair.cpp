/*
 * Applies RePair to the parenthesis bitstring of a tree.
 */

#include <iostream>
#include <string>

// Data Structures
#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

// Algorithms
#include "RePair/Coder.h"
#include "RePair/Prepair.h"
#include "RePair/RePair.h"

// Utils
#include "ArgParser.h"
#include "BPString.h"
#include "Timer.h"
#include "XML.h"


using std::cout;
using std::endl;
using std::string;

template <typename InType, typename DataType>
long long compress(vector<InType> &data, const std::string &description, const bool skipPrepair = false, const bool verbose = false) {
    Timer timer;
    cout << "RePair-ing the " << description;


    std::unordered_map<InType, InType> inputTransformations;
    if (!skipPrepair) {
        cout << ", preparing… " << flush;
        RePair::Prepair<InType>::prepare(data, inputTransformations);
        cout << timer.getAndReset() << "ms";
    }

    cout << ", initialising… " << flush;
    std::vector<DataType> output;
    RePair::RePair<DataType, InType> repair(data);
    cout << timer.getAndReset() << "ms, compressing… " << flush;

    repair.compress(output);
    RePair::Dictionary<DataType> &dictionary = repair.getDictionary();
    cout << "done (" << timer.getAndReset() << "ms)" << endl;

    cout << "Compressed representation has " << output.size() << " symbols, dictionary has " << dictionary.size() << " entries (" << dictionary.numSymbols() << " symbols)" << endl;

    if (verbose) {
        for (auto elem : output) {
            std::cout << elem << " ";
        }
        std::cout << std::endl << dictionary;
    }

    RePair::Coder<DataType> coder(output, dictionary);
    if (!skipPrepair) {
        coder.codeInputMapping(inputTransformations);
    }
    coder.compute();
    cout << coder.huff << " + " << coder.huff.getBitsForTableLabels() << " bits = " << (coder.getBitsNeeded() + 7) / 8 << " Bytes" << endl;
    return coder.getBitsNeeded();
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

    long long totalSize(0);
    totalSize += compress<bool, int>(bpstring, "tree structure", false, verbose);
    totalSize += compress<unsigned char, int>(labelnames, "labels", verbose);
    cout << "Output file needs " << totalSize << " bits (" << (totalSize + 7)/8 << " Bytes)" << endl;

    cout << "RESULT"
         << " file=" << filename
         << " compressed=" << totalSize
         << " bpstringbits=" << bpstring.size()
         << " labelstringits=" << labelnames.size() * 8
         << endl;
    return 0;
}
