#ifndef __PARAMS_ANALYSIS__
#define __PARAMS_ANALYSIS__

#include "stdio.h"
#include "string.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "main.h"

#include "compressFile.h"

int analyzeParams(int argc, char* argv[]);
int pack(int argc, char* argv[]);
int unpack(int argc, char* argv[]);
int getExample(int argc, char* argv[]);

#endif //__PARAMS_ANALYSIS__
