
#include "fileController.h"

FileController::FileController(std::string *input) {
	inputFileName = input;
}

bool FileController::openInputFile() {
	char *addr = &(*inputFileName)[0];

	if((fdin = open(addr, O_RDONLY)) < 0) {
		std::cout << "Cannot open " << inputFileName << " to read" << std::endl;
		return false;
	} else {
		if(fstat(fdin, &statbuf) < 0) {
			std::cout << "Cannot determine a size of the input file" << std::endl;
			return false;
		}
		return true;
	}
}

bool FileController::mmapInputFile() {
	if((src = (char*)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED)
		std::cout << "Error with mmap function!" << std::endl;
}

std::string FileController::getInputFileName() {
	return (*inputFileName);
}

char * FileController::getSrc() {
	return src;
}

// __________________________output file___________________________