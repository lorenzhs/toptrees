#pragma once

#include <cassert>
#include <map>
#include <sstream>
#include <vector>

using std::istringstream;
using std::map;
using std::string;
using std::vector;

/// Parse command-line arguments
/**
 * A simple command-line parser.
 *
 * Supports named arguments and switches as well as unnamed data arguments
 *
 * Example: ./foo -v -o outfolder in1.xml in2.xml
 */
class ArgParser {
public:
	/// Parse command-line arguments
	ArgParser(int argc, char** argv) {
		for (int i = 1; i < argc; ++i) {
			if (argv[i][0] == '-') {
				string arg(argv[i] + 1);
				if ((i + 1) < argc && argv[i + 1][0] != '-') {
					string val(argv[++i]);
					namedArgs[arg] = val;
				} else {
					namedArgs[arg] = "";
				}
			} else {
				dataArgs.push_back(argv[i]);
			}
		}
	}

	/// Get a named argument's value
	/// \param key the argument name
	/// \param defaultValue the value to return if the argument wasn't set
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

	/// check whether an argument was set
	bool isSet(const string &arg) const {
		return namedArgs.find(arg) != namedArgs.end();
	}

	/// the number of unnamed data arguments
	uint numDataArgs() const {
		return dataArgs.size();
	}

	/// get a data argument by its index (among the data arguments)
	string getDataArg(const int index) const {
		assert(0 <= index && index < (int)numDataArgs());
		return dataArgs[index];
	}

private:
	map<string, string> namedArgs;
	vector<string> dataArgs;
};