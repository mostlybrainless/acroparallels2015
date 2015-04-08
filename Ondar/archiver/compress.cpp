
#include "compress.h"
#include "constants.h"
#include <list>

/* This function is used to sort the leaves of huffmanTree by frequency */
static bool compare(Node *a, Node *b) {
	return (a->getFrequency() <= b->getFrequency());
}

Compress::Compress(std::string *input) {
	inputData = input;
}

boost::dynamic_bitset<>* Compress::getOutDataBitset() {
	return &outData;
}

/*
static void printHuffmanTable(std::unordered_map<char, std::pair<size_t, boost::dynamic_bitset<>>> huffmanTable) {
	for(auto elem : huffmanTable) {
		std::cout << "key :" << elem.first << " ";
		std::cout << "frequency :" << elem.second.first << " ";
		std::cout << "code :";

		for(boost::dynamic_bitset<>::size_type i = 0; i < elem.second.second.size(); i++)
			std::cout << elem.second.second[i];
		std::cout << "\n";
	}

}
*/

/*
static void printHuffmanTree(std::list<Node*>tree) {
	for(auto elem : tree) {
		std::cout << elem->leafValue << ' ';
		std::cout << elem->getFrequency() << std::endl;
	}
}
*/

// 
void Compress::createHuffmanTree() {
	std::list<Node*> tree;

	// firstly, the algorithm needs to add all elements of huffmanTable to our tree as leaves
	for(auto elemOfHuffmanTable : huffmanTable) {
		// create a new leaf with leafValue (first) and its frequency (second.first) in the inputData
		Node *leaf = new Node(elemOfHuffmanTable.first, elemOfHuffmanTable.second.first, LEAF);
		tree.push_back(leaf);
	}
	tree.sort(compare);
	
	// Now build the Huffman tree
	if(tree.size() == 1) {	// complexity of this member function size() is constant (C++11)
							// http://www.cplusplus.com/reference/list/list/size/
		Node *vertex = tree.front();
		tree.pop_front();

		Node *newVertex = new Node(vertex, nullptr, NOT_LEAF);
		tree.push_front(newVertex);

	} else {				// tree.size > 1
		while(tree.size() > 1) {
			Node *left = tree.front();
			tree.pop_front();

			Node *right = tree.front();
			tree.pop_front();

			Node *newVertex = new Node(left, right, NOT_LEAF);
			tree.push_front(newVertex);

			tree.sort(compare);
		}
	}
	
	// after that tree has only one vertex (which is a root)
	// Now set Huffman codes for each symbol (setting begins from the tree's root)
	tree.front()->setHuffmanCodes(huffmanTable);
}

void Compress::countFrequenciesOfEachSymbol() {
	size_t length = inputData->length();

	for(size_t i = 0; i < length; i++) {
		// add each element (which become a key value) of inputData to huffmanTable
		// after that increment frequency of this charValue
		// (frequency is the first of pair <frequency, huffmanCode>)
		++huffmanTable[(*inputData)[i]].first;
	}
}

// it's "concatenation": add bitset b to the end of bitset a
void static bitsetCat(boost::dynamic_bitset<>& a, boost::dynamic_bitset<> b) {
	for (size_t i = 0; i < b.size(); i++) {
		a.push_back(b[i]);
	}
}

// get bitset from value
static boost::dynamic_bitset<> toBitset(char value, size_t size) {
	boost::dynamic_bitset<> bitset(size, value);
	return bitset;
}

// This function writes info to 'outData' bitset:
// 1. in the first 64 bits of information about the size of the Huffman Table
// 2. after that each element of the Table (charValue, size of its Huffman code, its Huffman code)
void Compress::writeInfoAboutHuffmanTable() {
	
	outData.resize(SIZE_TABLE_INFO);

	char charValue;
	boost::dynamic_bitset<> huffmanCode;

	// add information about the table
	for(auto elem : huffmanTable) {
		charValue = elem.first;
		huffmanCode = huffmanTable[charValue].second;
		bitsetCat(outData, toBitset(charValue, SIZE_CHAR));
		bitsetCat(outData, toBitset(huffmanCode.size(), SIZE_HCODE));
		bitsetCat(outData, huffmanCode);
	}

	// add info about its size
	sizeHuffmanTable = outData.size() - SIZE_TABLE_INFO;
	for(size_t i = 0; i < SIZE_TABLE_INFO; i++) {
		outData[i] |= (sizeHuffmanTable >> i) & 1;
	}

}

void Compress::writeInputStringToOutData() {
	size_t len = inputData->length();

	for (size_t i = 0; i < len; i++) {
		// get the code for each symbol of the inputData
		// and write to the bitset outData
		bitsetCat(outData, huffmanTable[(*inputData)[i]].second);
	}
}

void Compress::encode() {
	// Firstly, the algorithm needs to get frequencies of each symbol of inputData
	countFrequenciesOfEachSymbol();

	// Secondly, create huffmanTree
	createHuffmanTree();
	//printHuffmanTable(huffmanTable);

	// Thirdly, write information about HuffmanTable (its size and elements) to outData
	writeInfoAboutHuffmanTable();

	// Finally, encode the input string
	writeInputStringToOutData();

	// uncomment to see the outDataBitset
/*
	for(boost::dynamic_bitset<>::size_type i = 0; i < outData.size(); i++)
		std::cout << outData[i];
	std::cout << "\n";
*/
}