#pragma once

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <ostream>
#include <fstream>
#include <vector>

struct DebugInfo {
	double generationDuration;
	double mergeDuration;
	double dagDuration;
	std::vector<double> edgeRatios;
	int numDagEdges;
	int numDagNodes;
	int height;
	double avgDepth;

	DebugInfo() : generationDuration(0.0), mergeDuration(0.0), dagDuration(0.0), edgeRatios(), numDagEdges(0), numDagNodes(0), height(0), avgDepth(0.0) {}

	double totalDuration() const {
		return generationDuration + mergeDuration + dagDuration;
	}

	void addEdgeRatio(double ratio) {
		edgeRatios.push_back(ratio);
	}

	double avgEdgeRatio() const {
		double result = 0.0;
		for (double ratio : edgeRatios) {
			result += ratio;
		}
		result /= edgeRatios.size();
		return result;
	}

	void add(const DebugInfo &other) {
		generationDuration += other.generationDuration;
		mergeDuration += other.mergeDuration;
		dagDuration += other.dagDuration;
		edgeRatios.insert(edgeRatios.end(), other.edgeRatios.begin(), other.edgeRatios.end());
		numDagEdges += other.numDagEdges;
		numDagNodes += other.numDagNodes;
		height += other.height;
		avgDepth += other.avgDepth;
	}

	void min(const DebugInfo &other) {
		generationDuration = std::min(generationDuration, other.generationDuration);
		mergeDuration = std::min(mergeDuration, other.mergeDuration);
		dagDuration = std::min(dagDuration, other.dagDuration);
		numDagEdges = std::min(numDagEdges, other.numDagEdges);
		numDagNodes = std::min(numDagNodes, other.numDagNodes);
		height = std::min(height, other.height);
		avgDepth = std::min(avgDepth, other.avgDepth);

		auto minHere = std::min_element(edgeRatios.begin(), edgeRatios.end());
		auto minThere = std::min_element(other.edgeRatios.begin(), other.edgeRatios.end());
		double minRatio = std::min(*minHere, *minThere);
		edgeRatios.resize(1);
		edgeRatios[0] = minRatio;
	}

	void max(const DebugInfo &other) {
		generationDuration = std::max(generationDuration, other.generationDuration);
		mergeDuration = std::max(mergeDuration, other.mergeDuration);
		dagDuration = std::max(dagDuration, other.dagDuration);
		edgeRatios.insert(edgeRatios.end(), other.edgeRatios.begin(), other.edgeRatios.end());
		numDagEdges = std::max(numDagEdges, other.numDagEdges);
		numDagNodes = std::max(numDagNodes, other.numDagNodes);
		height = std::max(height, other.height);
		avgDepth = std::max(avgDepth, other.avgDepth);

		auto maxHere = std::max_element(edgeRatios.begin(), edgeRatios.end());
		auto maxThere = std::max_element(other.edgeRatios.begin(), other.edgeRatios.end());
		double maxRatio = std::max(*maxHere, *maxThere);
		edgeRatios.resize(1);
		edgeRatios[0] = maxRatio;
	}

	void divide(const int factor) {
		generationDuration /= factor;
		mergeDuration /= factor;
		dagDuration /= factor;
		numDagEdges /= factor;
		numDagNodes /= factor;
		height /= factor;
		avgDepth /= factor;
	}

	void dump(std::ostream &os) const {
		os << totalDuration() << "\t"
		   << generationDuration << "\t"
		   << mergeDuration << "\t"
		   << dagDuration << "\t"
		   << avgEdgeRatio() << "\t"
		   << numDagEdges << "\t"
		   << numDagNodes << "\t"
		   << height << "\t"
		   << avgDepth
		   << std::endl;
	}

	void dumpEdgeRatioDistribution(std::ostream &os) const {
		for (auto it = edgeRatios.begin(); it != edgeRatios.end(); ++it) {
			os << *it << std::endl;
		}
	}

	static void dumpHeader(std::ostream &os) {
		os << "totalDuration" << "\t"
		   << "generationDuration" << "\t"
		   << "mergeDuration" << "\t"
		   << "dagDuration" << "\t"
		   << "edgeRatio" << "\t"
		   << "numDagEdges" << "\t"
		   << "numDagNodes" << "\t"
		   << "height" << "\t"
		   << "avgDepth"
		   << std::endl;
	}
};

struct Statistics {
	Statistics(const std::string filename = "") : numDebugInfos(0) {
		if (filename != "") {
			out.open(filename.c_str());
			assert(out.is_open());
		}
	}

	~Statistics() {
		if (out.is_open()) {
			out.close();
		}
	}

	void addDebugInfo(const DebugInfo &info) {
		if (out.is_open())
			info.dumpEdgeRatioDistribution(out);
		if (numDebugInfos == 0) {
			min = info;
			max = info;
			avg = info;
		} else {
			min.min(info);
			max.max(info);
			avg.add(info);
		}
		++numDebugInfos;
	}

	void compute() {
		avg.divide(numDebugInfos);
	}

	void dump(std::ostream &os) {
		os << std::fixed << std::setprecision(2);
		os << std::endl << "Statistics:" << std::endl << std::endl
		   << "Total duration p. tree: " << avg.totalDuration() << "ms (avg), " << min.totalDuration() << "ms (min), " << max.totalDuration() << "ms (max)" << std::endl
		   << "Random tree generation: " << avg.generationDuration << "ms (avg), " << min.generationDuration << "ms (min), " << max.generationDuration << "ms (max)" << std::endl
		   << "Top Tree construction:  " << avg.mergeDuration << "ms (avg), " << min.mergeDuration << "ms (min), " << max.mergeDuration << "ms (max)" << std::endl
		   << "Top DAG compression:    " << avg.dagDuration << "ms (avg), " << min.dagDuration << "ms (min), " << max.dagDuration << "ms (max)" << std::endl
		   << std::setprecision(6)
		   << "Edge comp. ratio: " << avg.avgEdgeRatio() << " (avg), " << min.avgEdgeRatio() << " (min), " << max.avgEdgeRatio() << " (max)" << std::endl
		   << std::setprecision(2)
		   << "DAG Edges: " << avg.numDagEdges << " (avg), " << min.numDagEdges << " (min), " << max.numDagEdges << " (max)" << std::endl
		   << "DAG Nodes: " << avg.numDagNodes << " (avg), " << min.numDagNodes << " (min), " << max.numDagNodes << " (max)" << std::endl
		   << "Tree height:    " << avg.height << " (avg), " << min.height << " (min), " << max.height << " (max)" << std::endl
		   << "Avg node depth: " << avg.avgDepth << " (avg), " << min.avgDepth << " (min), " << max.avgDepth << " (max)" << std::endl;
	}

	DebugInfo min, max, avg;
	std::ofstream out;
	int numDebugInfos;
};
