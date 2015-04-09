
#include "decompress.h"

Decompress::Decompress(boost::dynamic_bitset<>* input) {
	inputData = input;

	//std::cout << *inputData << std::endl;
}

static void bitsetCopy(boost::dynamic_bitset<>* from_bitset, size_t start, size_t amountCpBits, boost::dynamic_bitset<>* to_bitset) {
    size_t end = start + amountCpBits;

    for(size_t i = start; i < end; i++) {
        (*to_bitset)[i - start] = (*from_bitset)[i];
    }
}

void Decompress::readHuffmanTableSize() {
	boost::dynamic_bitset<> tmpbit_set(SIZE_TABLE_INFO, 0);

	bitsetCopy(inputData, 0, SIZE_TABLE_INFO, &tmpbit_set);
	sizeTable = tmpbit_set.to_ulong();
}

// This function reads Huffman Table from 'inputData' bitset
// and build its property huffmanTable
void Decompress::readHuffmanTable() {
	boost::dynamic_bitset<> charValue(SIZE_CHAR, 0);
	char value;
	boost::dynamic_bitset<> lengthHuffmanCode(SIZE_HCODE, 0);
	size_t lenHcode;
	boost::dynamic_bitset<> huffmanCode;

	size_t i = SIZE_TABLE_INFO;
	while(i < sizeTable) {
		bitsetCopy(inputData, i, SIZE_CHAR, &charValue);
		bitsetCopy(inputData, i + SIZE_CHAR, SIZE_HCODE, &lengthHuffmanCode);
		lenHcode = lengthHuffmanCode.to_ulong();

		huffmanCode.resize(lenHcode);
		huffmanCode <<= huffmanCode.size();

		/*
		// mdump
		if(huffmanCode.size() < lenHcode) {
			huffmanCode.resize(lenHcode - huffmanCode.size());
		}
		huffmanCode >>= huffmanCode.size();
		*/

		bitsetCopy(inputData, i + SIZE_CHAR + SIZE_HCODE, lenHcode, &huffmanCode);

		// now add a new element to HuffmanTable ( <charValue, hcode>, charValue is the key)
		value = (char)charValue.to_ulong();
		huffmanTable[value] = huffmanCode;
		i += SIZE_CHAR + SIZE_HCODE + lenHcode;

//		std::cout << value << ' ' << huffmanCode << std::endl;
	}
}

Node * Decompress::createHuffmanTree() {
	//Node *treeRoot = new Node(NOT_LEAF);
	;
	//
}

void Decompress::decode() {
	readHuffmanTableSize();
	readHuffmanTable();
	root = createHuffmanTree();

}