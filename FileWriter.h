#pragma once

#include <iostream>

#include "Timer.h"
#include "TreeSizeEstimation.h"

class FileWriter {
public:
	template <typename DataType>
	static long long write(const BinaryDag<DataType> &dag, const Labels<DataType> &labels, const std::string &fn, const bool verbose = true) {
		Timer timer;

		BitWriter writer(fn);
		DagEntropy<DataType> entropy(dag, labels, writer);
		entropy.calculate();

		if (verbose) std::cout
		     << "DAG Structure: " << entropy.dagStructureEntropy.huffman << endl
		     << "DAG Pointers:  " << entropy.dagPointerEntropy << endl
			 << "Merge Types:   " << entropy.mergeEntropy.huffman << endl
			 << "Label strings: " << entropy.labelDataEntropy.huffman << " + " << entropy.labelDataEntropy.getExtraSize() << " bits for symbols" << endl
			 << "Huffman calcuation took " << timer.getAndReset() << "ms; " << endl;

		//entropy.write();
		writer.close();

		//if (verbose) std::cout << "Wrote a total of " << writer.getBytesWritten() << " Bytes (" << writer.getBytesWritten()*8 << " bits) to " << fn << endl;

		return entropy.getTotalSize();
	}
};
