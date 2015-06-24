#pragma once

#include <cassert>
#include <unordered_map>
#include <sstream>
#include <vector>

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
		int pos = 1;
		while (pos < argc) {
			if (argv[pos][0] == '-') {
				// Read argument and advance
				std::string arg(&argv[pos++][1]);
				// check whether argument has a value (i.e. not just a flag)
				if (pos < argc && argv[pos][0] != '-') {
					namedArgs[arg] = std::string{argv[pos++]}; // assign and advance to next
				} else {
					namedArgs[arg] = "";
				}
			} else {
				dataArgs.push_back(std::string{argv[pos++]});
			}
		}
	}

	/// Get a named argument's value
	/// \param key the argument name
	/// \param defaultValue the value to return if the argument wasn't set
	template <typename T>
	T get(const std::string &key, const T defaultValue = T()) const {
		T retval;
		auto it = namedArgs.find(key);
		if (it != namedArgs.end()) {
			std::istringstream s(it->second);
			s >> retval;
		} else {
			// do this in the else case, otherwise empty string arguments
			// would return the default value instead of ""
			retval = defaultValue;
		}
		return retval;
	}

	/// check whether an argument was set
	bool isSet(const std::string &arg) const {
		return namedArgs.find(arg) != namedArgs.end();
	}

	/// the number of unnamed data arguments
	size_t numDataArgs() const {
		return dataArgs.size();
	}

	/// get a data argument by its index (among the data arguments)
	std::string getDataArg(const int index) const {
		assert(0 <= index && index < (int)numDataArgs());
		return dataArgs[index];
	}

protected:
	std::unordered_map<std::string, std::string> namedArgs;
	std::vector<std::string> dataArgs;
};
