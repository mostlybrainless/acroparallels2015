#include "defines.h"



#define COMPRESSION_TYPE Z_DEFAULT_COMPRESSION

int compressFile( char *source, char **dest, file_block* fb)
{
	*dest = (char *)malloc( fb->size_before + SIZE_OF_FILE_BLOCK);
	if ( *dest == NULL)
		RETURN_ERROR("compressFile: malloc error", -1);
	int ret = mycompress( source, &(*dest)[SIZE_OF_FILE_BLOCK], COMPRESSION_TYPE , fb);
	if (ret != Z_OK)
	{
		free( *dest);
		RETURN_ERROR("compressFile: compress error", -1);
	}
	makeFileWrapper( *dest, fb );
	return 0;
}

int decompressFile( char *source, char **dest )
{
	file_block  fb;
	parseFileBlock( source, &fb);
	*dest = (char *) malloc ( fb.size_before + SIZE_OF_FILE_BLOCK );
	if ( *dest == NULL)
		RETURN_ERROR("decompressFile: malloc error", -1);
	int ret = mydecompress( source + SIZE_OF_FILE_BLOCK, *dest , &fb);
	if (ret != Z_OK)
	{
		free( *dest);
		RETURN_ERROR("decompressFile: decompress error", -1);
	}
	return 0;
}

void makeFileWrapper(char *dest, file_block *fb )
{

	memcpy( dest, &fb->size_before, sizeof(fb->size_before));
	memcpy( dest + sizeof(fb->size_before), &fb->size_after, sizeof(fb->size_after));
	memcpy( dest + sizeof(fb->size_before) + sizeof(fb->size_after), fb->path, SIZE);
	memcpy( dest + sizeof(fb->size_before) + sizeof(fb->size_after) + SIZE, &fb->st_mode, sizeof(fb->st_mode));

	ALOGD("makeFileWrapper: size_before = %d", *(size_t *)(dest));
	ALOGD("makeFileWrapper: size_after = %d",  *(size_t *)(dest + sizeof(fb->size_before)));
	ALOGD("makeFileWrapper: path = %s", (dest + sizeof(fb->size_before) + sizeof(fb->size_after)));
	ALOGD("makeFileWrapper: st_mode = %d", *(mode_t *)(dest + sizeof(fb->size_before) + sizeof(fb->size_after) + SIZE));
}


void makeFileBlockStruct( file_block *dest, struct stat *source, char *path )
{
	dest->size_before = source->st_size;
	dest->st_mode = source->st_mode;
	strcpy(dest->path, path);
}

int getFileSizeAfter( char *source)
{
	return *(int *)(source + sizeof(size_t));
}

int getFileSizeBefore( char *source)
{
	return *(int *)(source);
}

char *getFilePath( char *source)
{
	return source + sizeof(size_t) + sizeof(size_t);
}

mode_t getFileMode( char *source)
{
	return *(mode_t *)(source + sizeof(size_t) + sizeof(size_t) + SIZE);
}

void parseFileBlock( char *source, file_block *fb)
{
	fb->size_before = getFileSizeBefore( source);
	fb->size_after = getFileSizeAfter( source);
	strcpy(fb->path, getFilePath( source));
	fb->st_mode = getFileMode( source);
}
