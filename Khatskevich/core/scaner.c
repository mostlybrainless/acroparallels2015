#define DEBUG

#include "archivator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <stddef.h>
#include <errno.h>

#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/mman.h>

#include <dirent.h> 
#include <libgen.h>


void takeNextFileFromDir(fInfoForCompressor* f_info);
void takeNextFileCancelDir(fInfoForCompressor * f_info);
void takeNextFileFromDir(fInfoForCompressor* f_info);
void takeNextFileNewDir(char* name, fInfoForCompressor * f_info);


static controllerMainStruct* controllerMainInfo;
fileIterStruct  fileIterInfo = {.fileIsProcessing = 0,.fileNameIter=0, .dirsNumber = 0,.offsetToRootFL = 0};


void scanerInit(controllerMainStruct* controllerInfo ){
    LOGMESG(LOG_INFO, "scanerInit, p =%p " ,controllerInfo  );
    controllerMainInfo = controllerInfo;
    fileIterInfo.dirsStack = (DirInterStruct*) malloc( sizeof( DirInterStruct) * MAXIMAL_STACK_OF_DIRECTORIES );
}


void scanerDestruct(){
    free(fileIterInfo.dirsStack);
}


fInfoForCompressor takeNextFile(char** names){
 
    LOGMESG(LOG_INFO, "takeNextFile");

    fInfoForCompressor f_info = {.valid = -1}; // will be returned
    int fd;
    
    if ( fileIterInfo.fileIsProcessing == 1){ //if iterator is in a directory
        takeNextFileFromDir(&f_info);
        goto takeNextFile_exit_ok;
    }
    if ( names[fileIterInfo.fileNameIter] == NULL){ // if it is the end 
        goto takeNextFile_exit_error_0;
    }
    

    char*  new_name = names[fileIterInfo.fileNameIter];
    fileIterInfo.fileNameIter++;


    struct stat st;
    if(stat(new_name,&st) == 0){ // If it is a directory
        if( S_ISDIR(st.st_mode) )
        {
            fileIterInfo.fileIsProcessing = 1; 
            takeNextFileNewDir(new_name, &f_info );
            goto takeNextFile_exit_ok;
        }
        /*if(st.st_mode & S_IFREG == 0)
        {
            goto takeNextFile_exit_error_0;
        }*/
    }
    
    fd = open( new_name , O_RDONLY);
    if ( fd == -1 ){
        LOGMESG(LOG_ERROR, "opening next file name %s", new_name);
        goto takeNextFile_exit_error_1;
    }
    size_t offset = addFileToFL(fd, basename(new_name) , &fileIterInfo.offsetToRootFL);
    
    fDescription * f_desc = ( fDescription *) ( controllerMainInfo->mmap_start +offset);
    
    f_info.length = f_desc->st_size;
    f_info.fd = fd;
    f_info.firstFileChunkOffset = offset + offsetof( fDescription, firstFileChunkOffset );
    f_info.valid = 1;
    
    takeNextFile_exit_error_1:
    takeNextFile_exit_error_0:
    takeNextFile_exit_ok:
        return f_info;
}
void takeNextFileNewDir(char* name, fInfoForCompressor * f_info){
    int rc;
    LOGMESG(LOG_INFO, "takeNextFileNewDir");
    if (fileIterInfo.dirsNumber + 1 < MAXIMAL_STACK_OF_DIRECTORIES){
        char* pathToDir;
        if ( fileIterInfo.dirsNumber == 0){
            pathToDir = (char*) malloc( strlen( name ) +1);
            memcpy(pathToDir, name, strlen( name ) +1);
        }else{
            char* pathToPrevDir = fileIterInfo.dirsStack[fileIterInfo.dirsNumber -1 ].pathToDir;
            pathToDir = (char*) malloc( strlen( pathToPrevDir) + strlen(name) +2);
            rc = snprintf( pathToDir,strlen( pathToPrevDir) + strlen(name) +2, "%s/%s",pathToPrevDir, name );
            if ( strlen(pathToDir) != strlen( pathToPrevDir) + strlen(name) +1) LOGMESG(LOG_ERROR, "strlen(pathToDir) != strlen( pathToPrevDir) + strlen(name) +1");
        }
        LOGMESG(LOG_INFO, "Path to finded directory -> %s", pathToDir);
        fileIterInfo.dirsStack[fileIterInfo.dirsNumber].pathToDir =pathToDir;
        
        fileIterInfo.dirsStack[fileIterInfo.dirsNumber].d = opendir(pathToDir);
        if (fileIterInfo.dirsStack[fileIterInfo.dirsNumber].d == NULL){
            LOGMESG(LOG_ERROR, "Opening directory ERROR! name = %s", pathToDir );
            perror("");
            free(pathToDir);
            f_info->valid = -1;
            fileIterInfo.fileIsProcessing = 0; 
        }
        size_t offset_to_fl;
        if ( fileIterInfo.dirsNumber == 0){
            LOGMESG(LOG_INFO, "Finded directory name -> %s", basename(name) );
            offset_to_fl = addDirToFL( 0 , basename(name) , &fileIterInfo.offsetToRootFL );
        }else{
            offset_to_fl = addDirToFL( 0 , name , &(fileIterInfo.dirsStack[fileIterInfo.dirsNumber -1 ].offset_to_fl) );
        }
        fileIterInfo.dirsStack[fileIterInfo.dirsNumber].offset_to_fl = offset_to_fl;

        fileIterInfo.dirsNumber++;
        takeNextFileFromDir(f_info);
    }else{
        LOGMESG(LOG_ERROR, "MAXIMAL_STACK_OF_DIRECTORIES overflow");
        f_info->valid = -1;
        fileIterInfo.fileIsProcessing = 0;
    }
}

void takeNextFileFromDir(fInfoForCompressor* f_info){
    LOGMESG(LOG_INFO, "takeNextFileFromDir");
    DIR *d;
    d = fileIterInfo.dirsStack[fileIterInfo.dirsNumber-1].d;
    if( d == NULL) LOGMESG(LOG_ERROR, "takeNextFileFromDir d = NULL!!!!!!!!!!!!!!!!!");
    struct dirent dirs;
    struct dirent *dir = &dirs;
    struct dirent **dirpp = &dir;
    readdir_r(d,dir, dirpp );
    if( dir == NULL){
        takeNextFileCancelDir(f_info);
        return;
    }
    if ( dir->d_type == DT_DIR){ //////////////////////////////////////////////////////// 
        if ( strcmp(dir->d_name , ".")==0 ||  strcmp(dir->d_name , "..")==0 ){
            takeNextFileFromDir(f_info);
            return;
        }else{
            takeNextFileNewDir(dir->d_name , f_info );
            return ;
        }
    }else if ( dir->d_type == DT_REG ||  dir->d_type == DT_LNK )   {// simple file
        char* name = dir->d_name;
        char * pathToDir = fileIterInfo.dirsStack[fileIterInfo.dirsNumber-1].pathToDir;
        char* pathToFile = (char*) malloc( strlen( pathToDir) + strlen(name) +2);
        snprintf( pathToFile,strlen( pathToDir) + strlen(name) +2, "%s/%s",pathToDir, name );
        int fd;
        if (  dir->d_type == DT_LNK )
        {
            fd = -1;
        }else{
            fd = open( pathToFile,O_RDONLY );
        }
        size_t offset = addFileToFL(fd,name, &(fileIterInfo.dirsStack[fileIterInfo.dirsNumber-1].offset_to_fl) );
        LOGMESG(LOG_INFO, "Path to finded file -> %s", pathToFile);
        fDescription * f_desc = ( fDescription *) ( controllerMainInfo->mmap_start +offset);
        LOGMESG(LOG_INFO, "Path to finded file -> %s, stmode = %o, d_type = %o", pathToFile,(int) f_desc->st_mode , ( int)dir->d_type );
        if(dir->d_type == DT_LNK ){
            LOGMESG(LOG_INFO, "Symlink processing!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            struct stat sb;
            lstat(pathToFile, &sb);
            f_desc->st_mode = S_IFLNK;
            f_desc->st_size = sb.st_size;
            char* linkname = (char*) malloc ( f_desc->st_size +  1);
            ssize_t r = readlink(pathToFile, linkname, f_desc->st_size + 1);
            if ( (r > f_desc->st_size) || r<0 ){//!
                LOGMESG(LOG_ERROR, "Symlink was changed while processing");
                f_desc->firstFileChunkOffset = 0;
            }else{
                linkname[r] = '\0';
                f_desc->firstFileChunkOffset = writeToOut( linkname , f_desc->st_size + 1 );
            }
            free(linkname);
            free(pathToFile);
            //close(fd); not necessary for links
            takeNextFileFromDir(f_info );
            return;
        }
        
        free(pathToFile);
        
        if (f_desc->st_size == 0){
            close(fd);
            takeNextFileFromDir(f_info);
            return;
        }
            
        
        f_info->length = f_desc->st_size;
        f_info->fd = fd;
        f_info->firstFileChunkOffset = offset + offsetof( fDescription, firstFileChunkOffset );
        f_info->valid = 1;
        return;
    }else{
        takeNextFileFromDir(f_info);
        return;
    }
} 

void takeNextFileCancelDir(fInfoForCompressor * f_info){
    LOGMESG(LOG_INFO, "takeNextFileCancelDir");
    if (fileIterInfo.dirsNumber > 0 ){
        fileIterInfo.dirsNumber--;
        char* pathToDir = fileIterInfo.dirsStack[fileIterInfo.dirsNumber].pathToDir;
        LOGMESG(LOG_WARN, "Path to canceling directory -> %s", pathToDir);
        free(pathToDir);
        closedir( fileIterInfo.dirsStack[fileIterInfo.dirsNumber].d );
    }
    if ( fileIterInfo.dirsNumber > 0){
        takeNextFileFromDir(f_info);
        return;
    }else{
        f_info->valid = 0;
        fileIterInfo.fileIsProcessing = 0;
        return;
    }
}
