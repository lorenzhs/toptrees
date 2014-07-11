#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

template <typename RNG>
class RandomTreeGenerator {
public:
	RandomTreeGenerator(RNG &gen) : generator(gen), distribution(0.0, 1.0) {}

	// Knuth random sampling algorithm from page 137 of
	// D.E. Knuth, Semi-numerical Algorithms, The Art of Computer Programming, Vol. 2 (Addision-Wesley, Reading, MA, 2nd ed., 1981).
	void selectionSampling(vector<bool> &result, const int n, const int N) {
		int t(0), m(0);
		double U;
		result.assign(N, false);
		while (m < n) {
			U = distribution(generator);
			if (((N - t) * U) < (n - m)) {
				m++;
				result[t] = true;
			}
			t++;
		}
	}

	void randomBalancedParenthesisBitstring(vector<bool> &result, const int numNodes) {
		vector<bool> sequence;
		selectionSampling(sequence, numNodes, 2 * numNodes);
		result.assign(numNodes * 2, false);

		phi<vector<bool>>(sequence.cbegin(), sequence.cend(), result.begin());

		assert(isWellFormed<vector<bool>>(result.begin(), result.end()));
	}

	template <typename TreeType>
	void generateTree(TreeType &tree, const int numNodes, const bool verbose = false) {
		vector<bool> bitstring;
		// numNodes - 1 because the root is not included
		randomBalancedParenthesisBitstring(bitstring, numNodes - 1);

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

private:
	template <typename T, typename TreeType>
	static typename T::const_iterator createTree(typename T::const_iterator begin, typename T::const_iterator end,
												 const int parentId, TreeType &tree) {
		if (*begin == false) {
			return ++begin;
		}
		while (begin < end) {
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

	// transform a balanced word into a well-formed balanced word. Algorithm from
	// Atkinson, Michael D., and J-R. Sack. "Generating binary trees at random." Information Processing Letters 41.1 (1992): 21-23.
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

	// is [begin, end) balanced, i.e. same number of opening and closing parentheses?
	static bool isBalanced(vector<bool>::const_iterator begin, vector<bool>::const_iterator end) {
		// an odd-length sequence can't be balanced
		if ((end - begin) % 2 == 1) {
			return false;
		}
		int count(0);
		for (auto it = begin; it < end; ++it) {
			count += *it;
		}
		return (count * 2) == (end - begin);
	}

	// is [begin, end) well-formed?
	template <typename T>
	static bool isWellFormed(typename T::const_iterator begin, typename T::const_iterator end) {
		int balance(0);
		for (typename T::const_iterator it = begin; it < end; ++it) {
			balance += *it ? 1 : -1;
			if (balance < 0) return false;
		}
		return true;
	}

	// separate [begin, end) into [begin, result) (irreducible) and [result, end)
	template <typename T>
	static typename T::const_iterator reducibleIndex(typename T::const_iterator begin, typename T::const_iterator end) {
		assert(isBalanced(begin, end));
		int delta(0);
		for (typename T::const_iterator it = begin; it < end; ++it) {
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