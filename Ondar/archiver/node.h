
#ifndef NODE_H
#define NODE_H

#include <boost/dynamic_bitset.hpp>
#include <unordered_map>
#include <iostream>

enum node_t {
	LEAF = 0,			// It has a leafValue be equal to charValue of a symbol of inputData
	NOT_LEAF			// This type for binding Node (its leafValue = 0)
};

class Node {
private:
	node_t nodeType;
	char leafValue;							// value of symbol
	size_t frequency;						// frequency of the leafValue
	boost::dynamic_bitset<> huffmanCode;	// Huffman code for the leafValue
	Node *left;								// left child
	Node *right;
public:
	Node(char inputValue, size_t inpFrequency, node_t type);	// for type LEAF
	Node(Node *leftNode, Node *rightNode, node_t type);			// it's for NOT_LEAF type
	~Node();
	size_t getFrequency();
	void setHuffmanCodes(std::unordered_map<char, std::pair<size_t, boost::dynamic_bitset<>>>& huffmanTable);
	//boost::dynamic_bitset<> getHuffmanCode();

};

#endif