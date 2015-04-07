#ifndef _ARCHIVATOR_SCANER_
#define _ARCHIVATOR_SCANER_
#include "archivator.h"
#include <dirent.h> 

typedef struct{
    DIR           *d;
    char* pathToDir;
    size_t offset_to_fl;
} DirInterStruct;


typedef struct{
    size_t fileNameIter; // iterator for files, taken from cmdl
    int fileIsProcessing; // show, if it is necessary to process next cmdl argument
    size_t dirsNumber;
    size_t offsetToRootFL;
    DirInterStruct* dirsStack;
} fileIterStruct;





void scanerInit(controllerMainStruct* controllerInfo );
fInfoForCompressor takeNextFile(char** names);


#endif //_ARCHIVATOR_SCANER_
