#pragma once

#include <iostream>
#include <map>

#include "3rdparty/rapidxml.hpp"
#include "3rdparty/rapidxml_utils.hpp"

using std::cout;
using std::endl;
using std::flush;

template<typename TreeType>
class XmlParser {
public:
	XmlParser(const std::string& fn, TreeType &t, std::map<int, std::string> &l): filename(fn), tree(t), labels(l) {}

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
		labels[rootId] = root->name();
		/*
		parseLinearSweep(root, rootId);
		/*/
		parseStructure(root, rootId);
		//*/

		if (verbose) cout << "building tree " << timer.elapsedMillis() << "ms" << endl;
	}

private:
	void parseLinearSweep(rapidxml::xml_node<>* node, int id) {
		rapidxml::xml_node<>* child = node->first_node();
		int childId;
		while (child != NULL) {
			childId = tree.addNode();
			tree.addEdge(id, childId);
			labels[childId] = child->name();
			parseLinearSweep(child, childId);
			child = child->next_sibling();
		}
	}

	void parseStructure(rapidxml::xml_node<>* node, int id) {
		rapidxml::xml_node<>* child = node->first_node();
		int numChildren(0), childId;
		while (child != NULL) {
			numChildren++;
			childId = tree.addNode();
			tree.addEdge(id, childId);
			labels[childId] = child->name();
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
	std::map<int, std::string> &labels;
};
