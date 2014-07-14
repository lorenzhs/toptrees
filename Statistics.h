#pragma once

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <mutex>
#include <ostream>
#include <fstream>
#include <vector>


struct StatWriter {
	static void open(const std::string &filename) {
		mutex.lock();
		out.open(filename.c_str());
		mutex.unlock();
	}

	static void close() {
		mutex.lock();
		if (out.is_open())
			out.close();
		mutex.unlock();
	}

	template <typename T>
	static void write(const T &data) {
		mutex.lock();
		if (out.is_open())
			out << data;
		mutex.unlock();
	}

	static std::mutex mutex;
	static std::ofstream out;
};

// Need to instantiate these, ugly
std::mutex StatWriter::mutex;
std::ofstream StatWriter::out;

struct DebugInfo {
	double generationDuration;
	double mergeDuration;
	double dagDuration;
	double minEdgeRatio;
	double maxEdgeRatio;
	double edgeRatios;
	short numEdgeRatios;
	int numDagEdges;
	int numDagNodes;
	int height;
	double avgDepth;

	DebugInfo()
		: generationDuration(0.0),
		  mergeDuration(0.0),
		  dagDuration(0.0),
		  minEdgeRatio(9.99),
		  maxEdgeRatio(0.0),
		  edgeRatios(0.0),
		  numEdgeRatios(0),
		  numDagEdges(0),
		  numDagNodes(0),
		  height(0),
		  avgDepth(0.0) {
	}

	double totalDuration() const {
		return generationDuration + mergeDuration + dagDuration;
	}

	void addEdgeRatio(double ratio) {
		++numEdgeRatios;
		edgeRatios += ratio;
		if (ratio < minEdgeRatio) {
			minEdgeRatio = ratio;
		}
		if (ratio > maxEdgeRatio) {
			maxEdgeRatio = ratio;
		}
		StatWriter::write(ratio);
	}

	double avgEdgeRatio() const {
		return edgeRatios / numEdgeRatios;
	}

	void add(const DebugInfo &other) {
		generationDuration += other.generationDuration;
		mergeDuration += other.mergeDuration;
		dagDuration += other.dagDuration;
		edgeRatios += other.edgeRatios;
		numEdgeRatios += other.numEdgeRatios;
		numDagEdges += other.numDagEdges;
		numDagNodes += other.numDagNodes;
		height += other.height;
		avgDepth += other.avgDepth;
	}

	void min(const DebugInfo &other) {
		generationDuration = std::min(generationDuration, other.generationDuration);
		mergeDuration = std::min(mergeDuration, other.mergeDuration);
		dagDuration = std::min(dagDuration, other.dagDuration);
		minEdgeRatio = std::min(minEdgeRatio, other.minEdgeRatio);
		numDagEdges = std::min(numDagEdges, other.numDagEdges);
		numDagNodes = std::min(numDagNodes, other.numDagNodes);
		height = std::min(height, other.height);
		avgDepth = std::min(avgDepth, other.avgDepth);
	}

	void max(const DebugInfo &other) {
		generationDuration = std::max(generationDuration, other.generationDuration);
		mergeDuration = std::max(mergeDuration, other.mergeDuration);
		dagDuration = std::max(dagDuration, other.dagDuration);
		minEdgeRatio = std::max(maxEdgeRatio, other.maxEdgeRatio);
		numDagEdges = std::max(numDagEdges, other.numDagEdges);
		numDagNodes = std::max(numDagNodes, other.numDagNodes);
		height = std::max(height, other.height);
		avgDepth = std::max(avgDepth, other.avgDepth);
	}

	void divide(const int factor) {
		generationDuration /= factor;
		mergeDuration /= factor;
		dagDuration /= factor;
		edgeRatios /= factor;
		numDagEdges /= factor;
		numDagNodes /= factor;
		height /= factor;
		avgDepth /= factor;
	}

	void dump(std::ostream &os) const {
		os << totalDuration() << "\t" << generationDuration << "\t" << mergeDuration << "\t" << dagDuration << "\t"
		   << minEdgeRatio << "\t" << maxEdgeRatio << "\t" << avgEdgeRatio() << "\t" << numDagEdges << "\t"
		   << numDagNodes << "\t" << height << "\t" << avgDepth << std::endl;
	}

	static void dumpHeader(std::ostream &os) {
		os << "totalDuration" << "\t"
		   << "generationDuration" << "\t"
		   << "mergeDuration" << "\t"
		   << "dagDuration" << "\t"
		   << "minEdgeRatio" << "\t"
		   << "maxEdgeRatio" << "\t"
		   << "avgEdgeRatio" << "\t"
		   << "numDagEdges" << "\t"
		   << "numDagNodes" << "\t"
		   << "height" << "\t"
		   << "avgDepth" << std::endl;
	}
};

struct Statistics {
	Statistics(const std::string filename = "") : numDebugInfos(0) {
		if (filename != "") {
			StatWriter::open(filename);
		}
	}

	~Statistics() {
		StatWriter::close();
	}

	void addDebugInfo(const DebugInfo &info) {
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

	void dump(std::ostream &os) {
		os << std::fixed << std::setprecision(2);
		os << std::endl << "Statistics:" << std::endl << std::endl
		   << "Total duration p. tree: " << avg.totalDuration() << "ms (avg), " << min.totalDuration() << "ms (min), " << max.totalDuration() << "ms (max)" << std::endl
		   << "Random tree generation: " << avg.generationDuration << "ms (avg), " << min.generationDuration << "ms (min), " << max.generationDuration << "ms (max)" << std::endl
		   << "Top Tree construction:  " << avg.mergeDuration << "ms (avg), " << min.mergeDuration << "ms (min), " << max.mergeDuration << "ms (max)" << std::endl
		   << "Top DAG compression:    " << avg.dagDuration << "ms (avg), " << min.dagDuration << "ms (min), " << max.dagDuration << "ms (max)" << std::endl
		   << std::setprecision(6)
		   << "Edge comp. ratio: " << avg.avgEdgeRatio() << " (avg), " << min.minEdgeRatio << " (min), " << max.maxEdgeRatio << " (max)" << std::endl
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
