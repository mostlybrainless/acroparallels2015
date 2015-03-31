#ifndef _ARCHIVATOR_DATA_PRESENTATION_
#define _ARCHIVATOR_DATA_PRESENTATION_

#define NUMDER_OF_FILES_IN_LIST 1000

#include <sys/stat.h>
#include <time.h>

#define CHUNK ( 256*1024 )
typedef struct{
    size_t size_of_data;
    size_t nextFileChunk;
    //checksumm
    //data char*
} fileChunk;

typedef struct{
    size_t size_of_name;
    mode_t    st_mode;    /* protection */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    off_t     st_size;    /* total size, in bytes */
    //time_t    st_atime;   /* time of last access */
    //time_t    st_mtime;   /* time of last modification */
    //time_t    st_ctime;   /* time of last status change */
    size_t    firstFileChunkOffset;
    // some space for filename (char*)    
}fDescription;

typedef struct{
    int count;
    size_t fDescriptionOffset[NUMDER_OF_FILES_IN_LIST];
    size_t nextFLOffset;
}fileList;

#endif // _ARCHIVATOR_DATA_PRESENTATION_



