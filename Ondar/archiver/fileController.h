
#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <iostream>
#include <sys/stat.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

class FileController {
private:
	int fdin, fdout;
	char *src;
//	boost::dynamic_bitset<> *dst;
	char *dst;
	struct stat statbuf;
	std::string *inputFileName;		// the name of the file that will be archived
	std::string *outputFileName;
public:
	FileController(std::string *input);
//	FileController(std::string *input, std::string *output);

	bool openInputFile();	// this function open the file that will be archived
							// also get information about the input file, in other words, init "statbuf" property
							// return true, if OK, else false

	bool mmapInputFile();	// true if mapping is successful
	std::string getInputFileName();
	char *getSrc();
	//bool openOutputFile();
	//bool mmapOutputFile();
};

#endif