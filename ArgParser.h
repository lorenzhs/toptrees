#pragma once

#include <cassert>
#include <map>
#include <sstream>
#include <vector>

using std::istringstream;
using std::map;
using std::string;
using std::vector;

class ArgParser {
public:
	ArgParser(int argc, char** argv) {
		for (int i = 1; i < argc; ++i) {
			string arg(argv[i] + 1);
			if (argv[i][0] == '-') {
				if ((i + 1) < argc && argv[i + 1][0] != '-') {
					string val(argv[++i]);
					namedArgs[arg] = val;
				} else {
					namedArgs[arg] = "";
				}
			} else {
				dataArgs.push_back(arg);
			}
		}
	}

	template <typename T>
	T get(const string &key, const T defaultValue = T()) {
		T retval;
		if (namedArgs.find(key) != namedArgs.end()) {
			istringstream s(namedArgs[key]);
			s >> retval;
		} else {
			// do this in the else case, otherwise empty string arguments
			// would return the default value instead of ""
			retval = defaultValue;
		}
		return retval;
	}

	bool isSet(const string &arg) const {
		return namedArgs.find(arg) != namedArgs.end();
	}

	uint numDataArgs() const {
		return dataArgs.size();
	}

	string getDataArg(const int index) const {
		assert(0 <= index && index < (int)numDataArgs());
		return dataArgs[index];
	}

private:
	map<string, string> namedArgs;
	vector<string> dataArgs;
};