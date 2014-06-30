#pragma once

#include <iostream>
#include <vector>

#include "Timer.h"

#include "3rdparty/rapidxml.hpp"
#include "3rdparty/rapidxml_utils.hpp"

using std::cout;
using std::endl;
using std::flush;

template<typename TreeType>
class XmlParser {
public:
	XmlParser(const std::string& fn, TreeType &t, std::vector<std::string> &l): filename(fn), tree(t), labels(l) {}

	void parse(const bool verbose = true) {
		if (verbose) cout << "Parsing " << filename << "... " << flush;
		Timer timer;

		rapidxml::file<> file(filename.c_str());
		rapidxml::xml_document<> xml;
		if (verbose) cout << "reading took " << timer.getAndReset() << "ms " << flush;

		xml.parse<rapidxml::parse_no_data_nodes>(file.data());
		rapidxml::xml_node<>* root = xml.first_node();
		if (verbose) cout <<"parsing " << timer.getAndReset() << "ms " << flush;

		int rootId = tree.addNode();
		assert((int)labels.size() == rootId);
		labels.push_back(root->name());
		parseStructure(root, rootId);

		if (verbose) cout << "building tree " << timer.elapsedMillis() << "ms" << endl;
	}

private:
	void parseStructure(rapidxml::xml_node<>* node, int id) {
		rapidxml::xml_node<>* child = node->first_node();
		int numChildren(0), childId;
		while (child != NULL) {
			numChildren++;
			childId = tree.addNode();
			tree.addEdge(id, childId);
			assert((int)labels.size() == childId);
			labels.push_back(child->name());
			child = child->next_sibling();
		}
		if (numChildren == 0) return;
		childId -= numChildren - 1;
		child = node->first_node();
		while (child != NULL) {
			parseStructure(child, childId);
			childId++;
			child = child->next_sibling();
		}
	}

	std::string filename;
	TreeType &tree;
	std::vector<std::string> &labels;
};
