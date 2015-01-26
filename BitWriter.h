#pragma once

#include <bitset>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class BitWriter {
public:
	static const int buffersize = 1024;

	BitWriter(const std::string &fn): fn(fn), bufitem(0), itempos(7), bytesWritten(0) {
		out.open(fn, std::ios::binary | std::ios::out);
		memset(buffer, 0, buffersize);
	}

	~BitWriter() {
		if (out.is_open()) {
			out.close();
		}
	}

	void writeBits(int data, size_t length) {
		for (unsigned int i = length; i--;) {
			std::cout << "Processing bit " << i << " of " << length << ": ORing buffer[" << bufitem << "] with " << (((data >> i) & 0x1) << itempos) << " (itempos " << itempos << ")" << std::endl;
			buffer[bufitem] |= ((data >> i) & 0x1) << itempos;
			bufitem += (itempos == 0);
			itempos = (itempos - 1) & 0x7;

			if (bufitem == buffersize) {
				writeBuffer();
			}
		}
	}

	void writeBits(const std::vector<bool> &vec) {
		for (bool bit : vec) {
			buffer[bufitem] |= bit << itempos;
			bufitem += (itempos == 0);
			itempos = (itempos - 1) & 0x7;

			if (bufitem == buffersize) {
				writeBuffer();
			}
		}
	}

	void write() {
		bytesWritten += bufitem;
		out.write((char*)&buffer, bufitem);
		memset(buffer, 0, bufitem);
		bufitem = 0;
		itempos = 7;
	}

	void clear() {
		bufitem = 0;
		itempos = 7;
		memset(buffer, 0, bufitem);
	}

	void close() {
		out.close();
	}

	void dump() {
		for (unsigned int i = 0; i <= bufitem; i++) {
			std::cout << std::bitset<8>(buffer[i]) << " ";
		}
		std::cout << std::endl;
	}

	unsigned long long getBytesWritten() {
		return bytesWritten;
	}

protected:
	void writeBuffer() {
		bytesWritten += buffersize;
		out.write((char*)&buffer, buffersize);
		bufitem = 0;
		memset(buffer, 0, buffersize);
	}
	const std::string &fn;
	std::ofstream out;
	unsigned char buffer[buffersize];
	unsigned int bufitem;
	unsigned int itempos;
	unsigned long long bytesWritten;
};
