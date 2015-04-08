#ifndef _ARCH_H_
#define _ARCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "zlib.h"
#include "../log/log.h"

#define min(x,y) (((x)>(y))?(y):(x))

typedef struct {
    int block_num;
    mode_t mode;
} file_info_t;

typedef struct {
    int compr_size;
} block_info_t;

int compress_block(int fd, char * src, int length_src);
int compress_file(const char * file_from, const char * file_to);
int decompress_block(int fd, char * src);
int decompress_file(const char * file_from, const char * file_to);
int is_have_postfix(const char * name);
int main_arch(int argc, const char * argv[]);

#endif // _ARCH_H_
