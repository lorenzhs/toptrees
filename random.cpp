#include <iostream>
#include <string>

#include "RandomTree.h"

using std::cout;
using std::endl;

int main(int argc, char** argv) {
	int seed = argc > 1 ? std::stoi(argv[1]) : 12345678;
	RandomTreeGenerator rand;
	vector<bool> dist;
	rand.randomBalancedParenthesisBitstring(dist, 10, seed);
	for (bool b: dist) {
		cout << (b ? "(" : ")");
	}
	cout << endl;
}
