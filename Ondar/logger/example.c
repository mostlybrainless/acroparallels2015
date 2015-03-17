
/*
 * Example of using this logger
 */

#include <stdio.h>
#include "logger.h"

int main() {
	int k = 1;
	int *pointer = NULL;

	if( !initLog(INFO, "log_file.txt") ) {
		printf("Error with init log file\n");
		return 0;
	}
	LOG(INFO, "This is log file");
	LOG(INFO, "%s%d", "Value of var k is equal to ", k);
	if(!pointer) {
		LOG(ERROR, "Error: address of the pointer %p, its value is %p", &pointer, pointer);
	}
	closeLogFile();
	return 0;
}
