#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "my_log.h"
#include <zlib.h>
#include <unistd.h> 
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef _POSIX_MAPPED_FILES
#define _POSIX_MAPPED_FILES
#endif

#define NAME_SIZE 256
#define ADD_BYTES 12

int my_compress(char* file_name);
int my_decompress(char* file_name);

