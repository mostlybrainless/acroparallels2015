
#include "node.h"

#define SET_NODE_TYPE(type) (nodeType = (type))

#define SET_CODE(node, bitValue)						\
	do {												\
		(node)->huffmanCode = huffmanCode;				\
		(node)->huffmanCode.push_back(bitValue);		\
	} while(0);


// node constructor which has LEAF type
Node::Node(char inputValue, size_t inpFrequency, node_t type = LEAF) {
	SET_NODE_TYPE(type);
	leafValue = inputValue;
	frequency = inpFrequency;
	//left = right = NULL;
	left = right = nullptr;		// because it's the leaf!
}

// for node with NOT_LEAF type
Node::Node(Node *leftNode = nullptr, Node *rightNode = nullptr, node_t type = NOT_LEAF) {
	SET_NODE_TYPE(type);
	leafValue = 0;
	if(leftNode != nullptr && rightNode != nullptr) {
		// the 'frequency' property of this vertex is
		// a sum of corresponding properties of left and right nodes
		frequency = leftNode->getFrequency() + rightNode->getFrequency();
	}
	left = leftNode;
	right = rightNode;
}


Node::~Node() {
	/*
	delete left;
	delete right;
	*/
}


size_t Node::getFrequency() {
	return frequency;
}

void Node::setHuffmanCodes(std::unordered_map<char, std::pair<size_t, boost::dynamic_bitset<>>>& huffmanTable) {
	// if inputData consists of only one symbol
	if(huffmanTable.size() == 1) {		// complexity of size()-function is constant
										// http://www.cplusplus.com/reference/unordered_map/unordered_map/size/
		left->huffmanCode.push_back(false);
		huffmanTable[left->leafValue].second = left->huffmanCode;
		return;
	}

	if(nodeType == LEAF) {
		huffmanTable[leafValue].second = huffmanCode;
	} else {	// nodeType == NOT_LEAF
		SET_CODE(left, false);
		left->setHuffmanCodes(huffmanTable);
		SET_CODE(right, true);
		right->setHuffmanCodes(huffmanTable);
	}
}