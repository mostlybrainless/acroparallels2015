#ifndef __COMPRESS_FILE__
#define __COMPRESS_FILE__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "main.h"
#include "hashFunc.h"
#include "../include/zlib.h"

typedef struct _PrefixOfFile {
    mode_t st_mode;
    off_t st_size;
    int numbBlocks;
    off_t offsetNextBlock;
    off_t hash;
} PrefixOfFile;

int compressFile(int fdIn, int fdOut);
int uncompressFile(int fdIn, char* outFile);

#endif //__COMPRESS_FILE__
