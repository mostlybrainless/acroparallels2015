#include "defines.h"



/* 
	сжатие     ./test_map -c file
	распаковка ./test_map -d file
*/

int main( int argc, char **argv)
{
	if (argc == 3 && (strcmp(argv[1], "-c") == 0)) 
	{
		struct stat info;
		file_block fb;
		if ( init_log( A, "map.log") == -1)
		{
			shm_unlink( SHAREDMEM_FILENAME);
			return -1;
		}
		int fd = open( argv[2] , O_RDWR,  0777);
		char *mappedFile = mapFileToMemory( fd, &info);
		if ( mappedFile == NULL)
			RETURN_ERROR("mapFileToMemory error", -1);
		char *compressed;
		int ret;
		makeFileBlockStruct( &fb, &info, argv[2]);
		ret = compressFile( mappedFile, &compressed, &fb);
		if( ret != 0 )
			RETURN_ERROR( "comressFile", -1 );
		int afterSize = getFileSizeAfter( compressed);
		char name[20];												//test variant
		strcpy( name, argv[2]);
		strcat( name, ".arch");
		close(fd);
		fd = open( name, O_RDWR | O_CREAT, 0777);
		write( fd, compressed, afterSize + SIZE_OF_FILE_BLOCK);
		close(fd);
		ret = munmap(mappedFile, fb.size_before );
		if ( ret == -1 )
			RETURN_ERROR("main: munmap error", -1);
		free(compressed);
	}
	else if (argc == 3 && (strcmp(argv[1], "-d") == 0)) 
	{
		if ( init_log( A, "map.log") == -1)
		{
			shm_unlink( SHAREDMEM_FILENAME);
			return -1;
		}
		int fd = open( argv[2] , O_RDWR,  0777);
		struct stat info;
		char *mappedFile, *dest;
		mappedFile = mapFileToMemory( fd, &info);
		if ( mappedFile == NULL)
			RETURN_ERROR("mapFileToMemory error", -1);
		int ret =  decompressFile( mappedFile, &dest);
		if( ret != 0 )
			RETURN_ERROR( "decomressFile", -1 );
		close(fd);
		fd = open( basename(getFilePath(mappedFile)), O_RDWR | O_CREAT, 0777);
		write( fd, dest, getFileSizeBefore(mappedFile) );
		close(fd);
		free(dest);
		ret = munmap(mappedFile, info.st_size );
		if ( ret == -1 )
			RETURN_ERROR("main: munmap error", -1);
	}
	else
		printf(" Incorrect arguments\n");
	return 0;
}
