#ifndef _ARCHIVATOR_DATA_PRESENTATION_
#define _ARCHIVATOR_DATA_PRESENTATION_

#define NUMDER_OF_FILES_IN_LIST 30


#include "archivator.h"
#include <sys/stat.h>
#include <time.h>

typedef struct{
    size_t size_of_data;
    size_t nextFileChunk;
    //checksumm
    //data char*
} fileChunk;

typedef struct{
    size_t size_of_name;
    int is_dir;
    mode_t    st_mode;    /* protection */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    off_t     st_size;    /* total size, in bytes */
    //time_t    st_atime;   /* time of last access */
    //time_t    st_mtime;   /* time of last modification */
    //time_t    st_ctime;   /* time of last status change */
    size_t    firstFileChunkOffset;
    // or if it iz dir-> it is pointer to FL
    // or if it is symlink -> address of symlink
    
    // some space for filename (char*)    
}fDescription;


typedef struct{
    int valid;
    int fd;
    size_t length;
    size_t firstFileChunkOffset;
}fInfoForCompressor;


typedef struct{
    int count;
    size_t fDescriptionOffset[NUMDER_OF_FILES_IN_LIST];
    size_t nextFLOffset;
}fileList;


typedef struct{
    long controllerID;          /* Message type */
    long from;
    int compression_lvl;
    size_t number;
    char* data_decomp;
    size_t size_decomp;
    char* data_comp;
    size_t size_comp;

}mesgStruct;

typedef struct{
    char* mmap_start;
    size_t mmap_size;
    size_t mmap_last_symbol;
    int number_of_scaners;
    int number_of_workers;
    int compression_lvl; // 1-7
    char** files;
    int fd_out;
    char* progname;
    int workers_qid;
    int sem;
} controllerMainStruct;



size_t addFileToFL(int fd,char* nextFileName, size_t* FLoffset  );
size_t addDirToFL(int fd, char* nextDirName , size_t* FLoffset );
int dataRestore(char* ofname, char* filename);
size_t writeToOut( char* data, size_t size );
int compressionPerform();
int dataPresentationControllerInit(char* outFName,
                                   int compression_lvl, int number_of_scaners, int number_of_workers, char ** files, char* progname);


#endif // _ARCHIVATOR_DATA_PRESENTATION_



