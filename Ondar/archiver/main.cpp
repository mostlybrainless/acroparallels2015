
// archiver

#include <iostream>
#include "compress.h"
#include "fileController.h"
#include "decompress.h"

int main() {
	std::string inputFileName = "input.txt";
	FileController fc = FileController(&inputFileName);
	fc.openInputFile();
	fc.mmapInputFile();

	std::string s;
	s = (std::string)fc.getSrc();
	
	Compress huffmanCompress = Compress(&s);
	huffmanCompress.encode();

	Decompress decompress = Decompress(huffmanCompress.getOutDataBitset());
	return 0;
}
