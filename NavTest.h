#pragma once

#include <iostream>

#include "Navigation.h"
#include "TopDag.h"

/// Traverse an in-memory Top DAG in preorder
template <typename DataType>
class PreorderTraversal {
public:
	PreorderTraversal(const TopDag<DataType> &dag, const bool print=false) : nav(dag), print(print) {}

	/// Do the traversal and print an XML representation to stdout
    std::pair<unsigned long long, unsigned long long> run() {
		openTag(0);
        auto visited = traverse(0);
        return std::make_pair(visited, nav.getMaxTreeStackSize());
    }

    /// Do the traversal and print an XML representation to stdout
    std::pair<unsigned long long, unsigned long long> runRight() {
        openTag(0);
        auto visited = traverseRight(0);
        return std::make_pair(visited, nav.getMaxTreeStackSize());
    }

protected:
	/// Output an opening tag
	void openTag(int depth=0, const bool newline=true) {
		if (!print) return;
		for (int i = 0; i < depth; ++i) cout << " ";
		std::cout << "<" << *nav.getLabel() << ">";
		if (newline) std::cout << std::endl;
	}

	/// Output a closing tag
	void closeTag(int depth=0, const bool indent=true) {
		if (!print) return;
		if (indent) for (int i = 0; i < depth; ++i) cout << " ";
		std::cout << "</" << *nav.getLabel() << ">" << std::endl;
	}

	/// Recursively traverse
    unsigned long long traverse(int depth=0) {
        unsigned long long visited = 0;
		if (!nav.isLeaf()) {
            nav.firstChild();
            ++visited;
			openTag(++depth, !nav.isLeaf());
		} else {
			closeTag(depth, !nav.isLeaf());
            if (nav.nextSibling()) {
                ++visited;
				openTag(depth, !nav.isLeaf());
			} else {
				while (!nav.nextSibling()) {
					bool hasParent = nav.parent();
					if (depth > 0) closeTag(--depth);
                    if (!hasParent) return visited;
                }
                ++visited;
				openTag(depth);
			}
		}
        return visited + traverse(depth);
    }

    /// Recursively traverse
    size_t traverseRight(int depth=0) {
        size_t visited = 0;
        if (!nav.isLeaf()) {
            nav.lastChild();
            ++visited;
            openTag(++depth, !nav.isLeaf());
        } else {
            closeTag(depth, !nav.isLeaf());
            if (nav.prevSibling()) {
                ++visited;
                openTag(depth, !nav.isLeaf());
            } else {
                while (!nav.prevSibling()) {
                    bool hasParent = nav.parent();
                    if (depth > 0) closeTag(--depth);
                    if (!hasParent) return visited;
                }
                ++visited;
                openTag(depth);
            }
        }
        return visited + traverseRight(depth);
    }

protected:
	Navigator<DataType> nav;
	const bool print;
};
