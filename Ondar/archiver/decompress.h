
#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "node.h"
#include "constants.h"

class Decompress {
private:
	std::unordered_map<char, boost::dynamic_bitset<>> huffmanTable;
	Node* root;
	boost::dynamic_bitset<>* inputData;	// input bitset was created by Compress
	size_t sizeTable;
	std::string *outData;
//
//
	void readHuffmanTableSize();	// from inputData
	void readHuffmanTable();

public:
	Decompress(boost::dynamic_bitset<>* input);
	Node* createHuffmanTree();
	void decode();
//
//
};

#endif