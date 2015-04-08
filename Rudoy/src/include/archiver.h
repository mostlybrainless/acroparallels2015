#include <libgen.h>

#pragma once

#define SIZE 256

#define SIZE_OF_FILE_BLOCK (SIZE + sizeof(size_t) + sizeof(size_t) + sizeof(mode_t))


typedef struct _file_block 
{
	size_t size_before;
	size_t size_after;
	char path[SIZE];
	mode_t st_mode;
}file_block;

int compressFile( char *source, char **dest, file_block* fb);
int decompressFile( char *source, char **dest );
void makeFileWrapper(char *dest, file_block *fb );
void makeFileBlockStruct( file_block *dest, struct stat *source, char *path );
int getFileSizeAfter( char *source);
void parseFileBlock( char *source, file_block *fb);
mode_t getFileMode( char *source);
char *getFilePath( char *source);
int getFileSizeBefore( char *source);
