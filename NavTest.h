#pragma once

#include <iostream>

#include "Navigation.h"
#include "TopDag.h"

template <typename DataType>
class PreorderTraversal {
public:
	PreorderTraversal(const TopDag<DataType> &dag, const bool print=false) : nav(dag), print(print) {}

	long long run() {
		openTag(0);
		traverse(0);
		return nav.getMaxTreeStackSize();
	}

protected:
	void openTag(int depth=0, const bool newline=true) {
		if (!print) return;
		for (int i = 0; i < depth; ++i) cout << " ";
		std::cout << "<" << *nav.getLabel() << ">";
		if (newline) std::cout << std::endl;
	}

	void closeTag(int depth=0, const bool indent=true) {
		if (!print) return;
		if (indent) for (int i = 0; i < depth; ++i) cout << " ";
		std::cout << "</" << *nav.getLabel() << ">" << std::endl;
	}

	void traverse(int depth=0) {
		if (!nav.isLeaf()) {
			nav.firstChild();
			openTag(++depth, !nav.isLeaf());
		} else {
			closeTag(depth, !nav.isLeaf());
			if (nav.nextSibling()) {
				openTag(depth, !nav.isLeaf());
			} else {
				while (!nav.nextSibling()) {
					bool hasParent = nav.parent();
					if (depth > 0) closeTag(--depth);
					if (!hasParent) return;
				}
				openTag(depth);
			}
		}
		traverse(depth);
	}

protected:
	Navigator<DataType> nav;
	const bool print;
};
