#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

/// Generates ordered unlabelled trees uniformly at random
template <typename RNG>
class RandomTreeGenerator {
public:
	/// Create a random tree generator
	/// \param gen the random generator to use
	RandomTreeGenerator(RNG &gen) : generator(gen), distribution(0.0, 1.0) {}

	// Knuth random sampling algorithm from page 137 of
	// D.E. Knuth, Semi-numerical Algorithms, The Art of Computer Programming, Vol. 2 (Addision-Wesley, Reading, MA, 2nd ed., 1981).
	void selectionSampling(vector<bool> &result, const int n, const int N) {
		int t(0), m(0);
		result.assign(N, false);
		while (m < n) {
			double U = distribution(generator);
			if (((N - t) * U) < (n - m)) {
				m++;
				result[t] = true;
			}
			t++;
		}
	}

	/// Generate a uniformly random balanced parenthesis bitstring, which defines a tree
	/// \param result a bool-vector for the output tree
	/// \param numNodes the number of nodes that the tree shall have
	void randomBalancedParenthesisBitstring(vector<bool> &result, const int numNodes) {
		vector<bool> sequence;
		selectionSampling(sequence, numNodes, 2 * numNodes);
		result.assign(numNodes * 2, false);

		phi<vector<bool>>(sequence.cbegin(), sequence.cend(), result.begin());

		assert(isWellFormed<vector<bool>>(result.begin(), result.end()));
	}

	/// Generate an unlabelled ordered tree uniformly at random
	/// \param tree a bool-vector for the output tree
	/// \param numEdges the number of edges that the tree shall have
	/// \param verbose whether to print the bitstring to stdout
	template <typename TreeType>
	void generateTree(TreeType &tree, const int numEdges, const bool verbose = false) {
		vector<bool> bitstring;
		// numEdges, because the root is not included in the bitstring
		randomBalancedParenthesisBitstring(bitstring, numEdges);

		if (verbose) {
			cout << "Bitstring: (";
			for (bool b : bitstring) {
				cout << (b ? "(" : ")");
			}
			cout << ")" << endl;
		}

		const int root = tree.addNode();
		createTree<vector<bool>>(bitstring.cbegin(), bitstring.cend(), root, tree);
		tree.compact(false);
	}

protected:
	/// Recursively create a tree from a parenthesis bitstring
	/// \param begin iterator to the beginning of the bitstring
	/// \param end iterator past the end of the bistring
	/// \param parentId parent node of the node to add
	/// \param tree the output tree -- node parentId needs to exist before calling this
	template <typename T, typename TreeType>
	static typename T::const_iterator createTree(typename T::const_iterator begin, typename T::const_iterator end,
												 const int parentId, TreeType &tree) {
		if (*begin == false) {
			return ++begin;
		}
		while (begin != end) {
			if (*begin == true) {
				int nodeId = tree.addNode();
				tree.addEdge(parentId, nodeId);
				++begin;
				begin = createTree<T, TreeType>(begin, end, nodeId, tree);
			} else {
				++begin;
				break;
			}
		}
		return begin;
	}

	/// transform a balanced word into a well-formed balanced word. Algorithm from
	/// Atkinson, Michael D., and J-R. Sack. "Generating binary trees at random." Information Processing Letters 41.1 (1992): 21-23.
	template <typename T>
	static typename T::iterator phi(typename T::const_iterator begin, typename T::const_iterator end,
									typename T::iterator outIt) {
		if (end <= begin) {
			return outIt;
		}

		typename T::const_iterator index = reducibleIndex<T>(begin, end);

		if (isWellFormed<T>(begin, index)) {
			outIt = std::copy(begin, index, outIt);
			outIt = phi<T>(index, end, outIt);
		} else {
			*outIt = 1;
			++outIt;
			outIt = phi<T>(index, end, outIt);
			*outIt = 0;
			++outIt;

			++begin;
			--index;
			while (begin != index) {
				*outIt = !*begin;
				++outIt; ++begin;
			}
		}

		return outIt;
	}

	/// check wether [begin, end) balanced, i.e. has same number of opening and closing parentheses
	/// \param begin defines the beginning of the sequence to check
	/// \param end defines the item past the end of the sequence to check
	/// \return true iff [begin, end) is balanced
	static bool isBalanced(vector<bool>::const_iterator begin, vector<bool>::const_iterator end) {
		// an odd-length sequence can't be balanced
		if ((end - begin) % 2 == 1) {
			return false;
		}
		int count(0);
		for (auto it = begin; it != end; ++it) {
			count += *it;
		}
		return (count * 2) == (end - begin);
	}

	/// is [begin, end) well-formed?
	template <typename T>
	static bool isWellFormed(typename T::const_iterator begin, typename T::const_iterator end) {
		int balance(0);
		for (typename T::const_iterator it = begin; it != end; ++it) {
			balance += *it ? 1 : -1;
			if (balance < 0) return false;
		}
		return true;
	}

	/// separate [begin, end) into [begin, result) (irreducible) and [result, end) so that
	/// [begin, end) is balanced
	template <typename T>
	static typename T::const_iterator reducibleIndex(typename T::const_iterator begin, typename T::const_iterator end) {
		assert(isBalanced(begin, end));
		int delta(0);
		for (typename T::const_iterator it = begin; it != end; ++it) {
			delta += *it ? 1 : -1;
			if (delta == 0) {
				return it + 1;
			}
		}
		return end;
	}

	RNG &generator;
	std::uniform_real_distribution<double> distribution;
};