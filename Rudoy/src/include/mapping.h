#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#include "archiver.h"

#pragma once




void* mapFileToMemory( int fd, struct stat *info );
int mycompress(char *source, char *dest, int level , file_block *info);
int mydecompress(char *source, char *dest , file_block *info);

