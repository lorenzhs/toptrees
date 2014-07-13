#pragma once

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <vector>

struct DebugInfo {
	double generationDuration;
	double mergeDuration;
	double dagDuration;
	double edgeRatio;
	int numDagEdges;
	int numDagNodes;
	int height;

	DebugInfo() : generationDuration(0.0), mergeDuration(0.0), dagDuration(0.0), edgeRatio(999.0), numDagEdges(0), numDagNodes(0), height(0) {}

	double totalDuration() const {
		return generationDuration + mergeDuration + dagDuration;
	}

	void addEdgeRatio(double ratio) {
		if (ratio < edgeRatio) {
			edgeRatio = ratio;
		}
	}

	void add(const DebugInfo &other) {
		generationDuration += other.generationDuration;
		mergeDuration += other.mergeDuration;
		dagDuration += other.dagDuration;
		edgeRatio += other.edgeRatio;
		numDagEdges += other.numDagEdges;
		numDagNodes += other.numDagNodes;
		height += other.height;
	}

	void min(const DebugInfo &other) {
		generationDuration = std::min(generationDuration, other.generationDuration);
		mergeDuration = std::min(mergeDuration, other.mergeDuration);
		dagDuration = std::min(dagDuration, other.dagDuration);
		edgeRatio = std::min(edgeRatio, other.edgeRatio);
		numDagEdges = std::min(numDagEdges, other.numDagEdges);
		numDagNodes = std::min(numDagNodes, other.numDagNodes);
		height = std::min(height, other.height);
	}

	void max(const DebugInfo &other) {
		generationDuration = std::max(generationDuration, other.generationDuration);
		mergeDuration = std::max(mergeDuration, other.mergeDuration);
		dagDuration = std::max(dagDuration, other.dagDuration);
		edgeRatio = std::max(edgeRatio, other.edgeRatio);
		numDagEdges = std::max(numDagEdges, other.numDagEdges);
		numDagNodes = std::max(numDagNodes, other.numDagNodes);
		height = std::max(height, other.height);
	}

	void divide(const int factor) {
		generationDuration /= factor;
		mergeDuration /= factor;
		dagDuration /= factor;
		edgeRatio /= factor;
		numDagEdges /= factor;
		numDagNodes /= factor;
		height /= factor;
	}

	void dump(std::ostream &os) const {
		os << totalDuration() << "\t"
		   << generationDuration << "\t"
		   << mergeDuration << "\t"
		   << dagDuration << "\t"
		   << edgeRatio << "\t"
		   << numDagEdges << "\t"
		   << numDagNodes << "\t"
		   << height
		   << std::endl;
	}

	static void dumpHeader(std::ostream &os) {
		os << "totalDuration" << "\t"
		   << "generationDuration" << "\t"
		   << "mergeDuration" << "\t"
		   << "dagDuration" << "\t"
		   << "edgeRatio" << "\t"
		   << "numDagEdges" << "\t"
		   << "numDagNodes" << "\t"
		   << "height"
		   << std::endl;
	}
};

struct Statistics {
	void addDebugInfo(const DebugInfo &info) {
		debugInfos.push_back(info);
	}

	void compute() {
		if (debugInfos.empty()) return;

		min = debugInfos[0];
		max = debugInfos[0];
		avg = DebugInfo();
		avg.edgeRatio = 0.0;

		for (DebugInfo &info : debugInfos) {
			min.min(info);
			max.max(info);
			avg.add(info);
		}
		avg.divide(debugInfos.size());
	}

	void dump(std::ostream &os) {
		os << std::fixed << std::setprecision(2);
		os << std::endl << "Statistics:" << std::endl << std::endl
		   << "Total duration p. tree: " << avg.totalDuration() << "ms (avg), " << min.totalDuration() << "ms (min), " << max.totalDuration() << "ms (max)" << std::endl
		   << "Random tree generation: " << avg.generationDuration << "ms (avg), " << min.generationDuration << "ms (min), " << max.generationDuration << "ms (max)" << std::endl
		   << "Top Tree construction:  " << avg.mergeDuration << "ms (avg), " << min.mergeDuration << "ms (min), " << max.mergeDuration << "ms (max)" << std::endl
		   << "Top DAG compression:    " << avg.dagDuration << "ms (avg), " << min.dagDuration << "ms (min), " << max.dagDuration << "ms (max)" << std::endl
		   << std::setprecision(6)
		   << "Worst edge comp. ratio: " << avg.edgeRatio << " (avg), " << min.edgeRatio << " (min), " << max.edgeRatio << " (max)" << std::endl
		   << std::setprecision(2)
		   << "DAG Edges: " << avg.numDagEdges << " (avg), " << min.numDagEdges << " (min), " << max.numDagEdges << " (max)" << std::endl
		   << "DAG Nodes: " << avg.numDagNodes << " (avg), " << min.numDagNodes << " (min), " << max.numDagNodes << " (max)" << std::endl
		   << "Tree height: " << avg.height << " (avg), " << min.height << " (min), " << max.height << " (max)" << std::endl;
	}

	std::vector<DebugInfo> debugInfos;
	DebugInfo min, max, avg;
};
