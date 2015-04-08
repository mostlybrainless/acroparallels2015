
#ifndef COMPRESS_H
#define COMPRESS_H

#include "node.h"

class Compress {
private:
	// elements of unordered_map huffmanTable:
	// 1) key value is char symbol
	// 2) mapped value is a pair of frequency and Huffman code of the symbol
	std::unordered_map<char, std::pair<size_t, boost::dynamic_bitset<>>> huffmanTable;
//	Node *root;
	std::string *inputData;
	boost::dynamic_bitset<> outData;
	size_t sizeHuffmanTable;

	void writeInfoAboutHuffmanTable();
	void writeInputStringToOutData();
public:
	Compress(std::string *inputString);
	void countFrequenciesOfEachSymbol();
	void createHuffmanTree();
	void encode();
	boost::dynamic_bitset<> *getOutDataBitset();
};

#endif