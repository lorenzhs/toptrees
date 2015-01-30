#pragma once

#include <iostream>

#include "Navigation.h"

template <typename DataType>
class PreorderTraversal {
public:
	PreorderTraversal(const BinaryDag<DataType> &dag) : nav(dag) {}

	long long run() {
		traverse();
		return nav.getMaxTreeStackSize();
	}

protected:
	void traverse() {
		if (!nav.isLeaf()) {
			nav.firstChild();
		} else if (nav.nextSibling()) {
			// already moved there, nothing to do here
		} else {
			while (!nav.nextSibling()) {
				bool hasParent = nav.parent();
				if (hasParent) {
				} else {
					return;
				}
			}
		}
		traverse();
	}

protected:
	Navigator<DataType> nav;	
};
