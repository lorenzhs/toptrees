#pragma once

#include <iostream>

#include "Navigation.h"

template <typename DataType>
class PreorderTraversal {
public:
	PreorderTraversal(const BinaryDag<DataType> &dag) : nav(dag) {}

	void run() {
		std::cout << "rt Label: " << *nav.getLabel() << std::endl;
		traverse();
	}

protected:
	void traverse() {
		if (!nav.isLeaf()) {
			nav.firstChild();
			std::cout << "fC Label: " << *nav.getLabel() << std::endl;
		} else if (nav.nextSibling()) {
			std::cout << "nS Label: " << *nav.getLabel() << std::endl;
		} else {
			while (!nav.nextSibling()) {
				bool hasParent = nav.parent();
				if (hasParent) {
					std::cout << "\tpa Label: " << *nav.getLabel() << std::endl;
				} else {
					std::cout << "DONE." << std::endl;
					return;
				}
			}
			std::cout << "nS Label: " << *nav.getLabel() << std::endl;
		}
		traverse();
	}

protected:
	Navigator<DataType> nav;	
};
