#define DEBUG

#include "archivator.h"
#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <pthread.h>
#include <stddef.h>
#include <assert.h> 
#include <errno.h>

#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/mman.h>

void* pthreadMainLoop(void*);
void* controllerCompress(void* ID);
size_t writeToOut( char* data, size_t size );

static controllerMainStruct controllerMainInfo; 

int sem_change_ptr(struct sembuf *sops, short num , short value , short flag)//helps for semaphoring
//Ptr, number of semaphore, value to encrease, flags
{
        sops[0].sem_flg = flag;
        sops[0].sem_num = num;
        sops[0].sem_op = value;
        return 0;
}


int dataPresentationControllerInit(char* outFName,
                                   int compression_lvl, int number_of_scaners, int number_of_workers, char ** files, char* progname)
{ 
    LOGMESG(LOG_INFO, "dataPresentationControllerInit");
    int fd_out, rc;
    char* mmap_start;
    if ( (number_of_scaners <= 0 ||  number_of_scaners > 100 ) ||  (number_of_workers <= 0 ||  number_of_workers > 100 ) ){ 
        LOGMESG(LOG_ERROR ,"Not valid nthread");
        goto dataPresentationControllerInit_exit_error_0;
    }
    controllerMainInfo.number_of_scaners = number_of_scaners;
    controllerMainInfo.number_of_workers = number_of_workers;
    if ( compression_lvl <= 0 ||  compression_lvl > 7){ 
        LOGMESG(LOG_ERROR ,"Not valid compression_lvl");
        goto dataPresentationControllerInit_exit_error_0;
    }
    
    controllerMainInfo.compression_lvl = compression_lvl;
    if ( files == NULL){
        LOGMESG(LOG_ERROR ,"Not valid files struct");
        goto dataPresentationControllerInit_exit_error_0;
    }
    controllerMainInfo.files = files;
    if ( progname == NULL){
        LOGMESG(LOG_ERROR ,"Not valid progname");
        goto dataPresentationControllerInit_exit_error_0;
    }
    controllerMainInfo.progname = progname;
    fd_out = open( outFName, O_CREAT | O_RDWR, 0666);
    if ( fd_out < 0){
        LOGMESG(LOG_ERROR ,"Error while opening output file");
        goto dataPresentationControllerInit_exit_error_0; 
    }
    
    controllerMainInfo.fd_out = fd_out;
    if (ftruncate( fd_out, MAX_FILE_LENGTH) ){
        LOGMESG(LOG_ERROR ,"Error while ftruncate output file");
        goto dataPresentationControllerInit_exit_error_1;
    }
    controllerMainInfo.mmap_size = MAX_FILE_LENGTH;
    
    mmap_start = mmap(NULL, MAX_FILE_LENGTH, PROT_READ | PROT_WRITE ,
                    MAP_SHARED, fd_out, 0);
    if ( mmap_start == MAP_FAILED){
        LOGMESG(LOG_ERROR ,"Error while creating mmap"); 
        goto dataPresentationControllerInit_exit_error_1;
    }
    controllerMainInfo.mmap_start = mmap_start;
    controllerMainInfo.mmap_last_symbol = 0;
    
    int qid;
    key_t msgkey = ftok(controllerMainInfo.progname, MESG_SEQ_KEY);
    if(( qid = msgget( IPC_PRIVATE, IPC_CREAT | 0666  )) == -1) {
                LOGMESG(LOG_ERROR,"open_queue for workers");
                goto dataPresentationControllerInit_exit_error_2;
    }
    controllerMainInfo.workers_qid = qid;
    //key_t semkey = ftok(IPC_PRIVATE, SEMAFORS_KEY );
    int sem = semget( IPC_PRIVATE , 8 , IPC_CREAT | 0666);
        if ( sem == -1 ) {
                LOGMESG(LOG_ERROR,"semget");
                perror("");
                goto dataPresentationControllerInit_exit_error_2;
        } 
    controllerMainInfo.sem = sem;
    struct sembuf sops[4]; 
    //need semundo
    sem_change_ptr(sops, S_TAKE_NEXT_FILE , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
    sem_change_ptr(sops+1, S_ADD_FILE_TO_FL , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
    sem_change_ptr(sops+2, S_WRITE_TO_OUT , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
    sem_change_ptr(sops+3, S_PMALLOC , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
    semop( controllerMainInfo.sem , sops, 4);

    fileList firstStructure;
    firstStructure.count = 0;
    firstStructure.nextFLOffset = 0;
    size_t offset;
    offset = writeToOut( (char*) &firstStructure, sizeof(fileList));
    
    scanerInit(&controllerMainInfo);
    
    return 0;
    dataPresentationControllerInit_exit_error_2:
        munmap( controllerMainInfo.mmap_start, controllerMainInfo.mmap_size);
    dataPresentationControllerInit_exit_error_1:
        close(fd_out);
    dataPresentationControllerInit_exit_error_0:
        return -1;
}

int compressionPerform(){
    int rc;
    pthread_t workerThreadId[controllerMainInfo.number_of_workers];
    pthread_t compressorThreadId[controllerMainInfo.number_of_scaners];
    int ID[controllerMainInfo.number_of_scaners];
    int IDw[controllerMainInfo.number_of_workers];
    int i;
      
    for ( i = 0; i < controllerMainInfo.number_of_workers ; i++){ 
        LOGMESG(LOG_INFO,"creating worker %d\n", i);
        IDw[i] = i;
        rc = pthread_create(&workerThreadId[i] , (void*) NULL, pthreadMainLoop, (void*) &IDw[i]);
    }
    for ( i = 0; i < controllerMainInfo.number_of_scaners ; i++){
        ID[i] = TYPE_WORKER + 1 + i;
        LOGMESG(LOG_INFO,"creating controller %d\n", i);
        rc = pthread_create(&compressorThreadId[i] , (void*) NULL, controllerCompress, (void*) &ID[i]);
    }
    for ( i = 0; i < controllerMainInfo.number_of_scaners ; i++){
        rc = pthread_join(compressorThreadId[i], NULL);
        LOGMESG(LOG_INFO,"closing controller %d\n", i);
    }
    LOGMESG(LOG_INFO, "Closing messages");
    msgctl( controllerMainInfo.workers_qid, IPC_RMID, NULL); 
    for ( i = 0; i < controllerMainInfo.number_of_workers ; i++){
        rc = pthread_join(workerThreadId[i], NULL);
        LOGMESG(LOG_INFO,"closing controller %d\n", i);
    }
    munmap( controllerMainInfo.mmap_start, controllerMainInfo.mmap_size);
    ftruncate( controllerMainInfo.fd_out, controllerMainInfo.mmap_last_symbol);
    close( controllerMainInfo.fd_out);
}

void* pthreadMainLoop( void* ID){
    int workerID = *((int*) ID);
    while( threadDoCompression() == 0 ){
        LOGMESG(LOG_INFO, "Worker # %d", workerID );
    };
    return ;
}

int threadDoCompression(){
    mesgStruct mesgInfo;
    if ( takeInfoMesg( &mesgInfo, TYPE_WORKER) ){
        return 1;
    }
    LOGMESG(LOG_INFO, "Compressing %d bytes for controller %d", (int) mesgInfo.size_decomp, mesgInfo.from );
    compressChunk(&mesgInfo);
    LOGMESG(LOG_INFO, "Compressing Finished for controller %d size comp = %d ,  data_decomp p = %p", mesgInfo.from , (int) mesgInfo.size_comp, mesgInfo.data_decomp  );
    mesgInfo.controllerID = mesgInfo.from;
    giveInfoMesg(&mesgInfo); // controller must free this memory
    return 0;
}
 
void* controllerCompress(void* ID){
    struct sembuf sops[2];

    int controllerID = *((int*) ID);
    while( 1)
    { 
        fInfoForCompressor nextFile ;
        LOGMESG(LOG_INFO, "Waiting for  S_TAKE_NEXT_FILE 0" );
        sem_change_ptr(sops, S_TAKE_NEXT_FILE , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
        sem_change_ptr(sops+1, S_TAKE_NEXT_FILE , 1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
        if ( semop( controllerMainInfo.sem , sops, 2) !=0 )  LOGMESG(LOG_ERROR, "semop !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        
        nextFile = takeNextFile(controllerMainInfo.files);
            LOGMESG(LOG_INFO, "Waiting for  S_TAKE_NEXT_FILE -1" );
            sem_change_ptr(sops, S_TAKE_NEXT_FILE , -1,SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
            semop( controllerMainInfo.sem , sops, 1);
        if ( nextFile.valid == -1) {

            return;
        }
        if ( nextFile.valid == 1) {
            compressFileByChunks(controllerID , nextFile);
            close(nextFile.fd);
        }
    }
}

int compressFileByChunks(int controllerID, fInfoForCompressor f_info){
    char* mmap_next_file;
    size_t length = f_info.length;
    int fd = f_info.fd;
    size_t placeForNextChunkOffset = f_info.firstFileChunkOffset ;
    

    mmap_next_file = mmap(NULL, length, PROT_READ ,
                    MAP_PRIVATE, fd, 0);
    if ( mmap_next_file == NULL){
        LOGMESG(LOG_ERROR ,"Error while creating mmap");
        goto compressFileByChunks_exit_error_0;
    }
    size_t sended = 0;
    size_t received = 0;
    size_t next_receiving = 0;
    size_t number_of_cicles = 0;
    mesgStack * stack = mesgStackInit();
    while ( next_receiving*CHUNK < length ){
        LOGMESG(LOG_INFO, "Controller sending work" );
        if ( sended*CHUNK < length){
            mesgStruct mesgInfo;
            mesgInfo.size_decomp = (length - sended*CHUNK < CHUNK) ? length - sended*CHUNK : CHUNK;
            mesgInfo.data_decomp = mmap_next_file+ sended*CHUNK;
            mesgInfo.compression_lvl = controllerMainInfo.compression_lvl;
            mesgInfo.controllerID = TYPE_WORKER;
            mesgInfo.number = sended;
            mesgInfo.from = controllerID;
            size_t destLen = MAX_COMP_CHUNK_LEN;
            mesgInfo.size_comp = destLen;
            mesgInfo.data_comp = (char*) malloc( destLen + sizeof(fileChunk) );
            LOGMESG(LOG_INFO, "Sended number %lld fron id = %d", (long long int) sended, controllerID );
            giveInfoMesg( &mesgInfo );
            sended++;
        } 
        if ( number_of_cicles >= FORA_IN_CHUNKS ){// when number will be more ther 1 , you should care about order.
            mesgStruct* mesgInfo ;
            if (received * CHUNK<length)
            {
                mesgInfo = (mesgStruct*) malloc(sizeof(mesgStruct));
                LOGMESG(LOG_INFO, "Controller takeing work" );
                takeInfoMesg(mesgInfo , controllerID );
                LOGMESG(LOG_INFO, "Controller taked work" );
                LOGMESG(LOG_INFO, "->Recieved number %lld, size = %d,  p = %p", (long long int) mesgInfo->number, (int)mesgInfo->size_comp  , mesgInfo);
                mesgStackAdd( stack, mesgInfo);
            }
            mesgInfo = mesgStackGetFirst(stack);
            
            
            if ( mesgInfo->number == next_receiving){
                next_receiving++;
                mesgStackDeleteFirst(stack);
                size_t offset = writeToOut(mesgInfo->data_comp,mesgInfo->size_comp);
                free(mesgInfo->data_comp);
                free(mesgInfo);
                writeToOutByOffset( (char*) &offset, sizeof(size_t) ,placeForNextChunkOffset);
                placeForNextChunkOffset = offset + offsetof( fileChunk, nextFileChunk);
            }
            received++;
        }
        number_of_cicles ++;
    }
    mesgStackDestruct(stack);
    
    compressFileByChunks_exit_error_1:
        munmap( mmap_next_file, length);
    compressFileByChunks_exit_error_0:
            return -1;
} 

int writeToOutByOffset(char* data, size_t size, size_t offset ){
    if ( offset + size > controllerMainInfo.mmap_size){
        return -1;
    }
    memcpy( controllerMainInfo.mmap_start + offset, data, size);
    return 0;
    
}

size_t writeToOut( char* data, size_t size ){
    LOGMESG(LOG_INFO, "Writing data to OUT size = %lld, p = %p", (long long) size, data  );
    //Need to be atomic    !!!!!!!!!!!!!!!!!!
    
        struct sembuf sops[2];
        sem_change_ptr(sops, S_WRITE_TO_OUT , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
        sem_change_ptr(sops+1, S_WRITE_TO_OUT , 1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
        semop( controllerMainInfo.sem , sops, 2);
    if (controllerMainInfo.mmap_size<controllerMainInfo.mmap_last_symbol+size){
        /*
        char *new_mapping = (char*) mremap((void*)controllerMainInfo.mmap_start, controllerMainInfo.mmap_size,
                                   controllerMainInfo.mmap_size + 10*1000*1000, MREMAP_MAYMOVE);
        if (new_mapping == MAP_FAILED){
            LOGMESG( LOG_ERROR, "mremap");
            return 0;
        }
        controllerMainInfo.mmap_start = new_mapping;*/
        LOGMESG(LOG_ERROR, "FILE BIGGER THAN IT WAS EXPECTED"); 
        //use int ftruncate(int fildes, off_t length);
    }
    
    size_t offset_to_new_data = controllerMainInfo.mmap_last_symbol;
    controllerMainInfo.mmap_last_symbol = controllerMainInfo.mmap_last_symbol + size;
    
    sem_change_ptr(sops, S_WRITE_TO_OUT , -1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the writeToOut function
    semop( controllerMainInfo.sem , sops, 1);
    
    //Need to be atomic    !!!!!!!!!!!!!!!!!!
    memcpy( controllerMainInfo.mmap_start + offset_to_new_data, data, size);// thread safe?
    return offset_to_new_data;
}
 

int takeInfoMesg( mesgStruct * mesgInfo , long controllerID ){
        int result, length;

        /* The length is essentially the size of the structure minus sizeof(mtype) */
        length = sizeof(mesgStruct) - sizeof(long);        
        result = msgrcv( controllerMainInfo.workers_qid , mesgInfo, length, controllerID ,  0);
        if(result  == -1)
        {
            LOGMESG( LOG_INFO , "msgrcving by controllerID %d" ,controllerID);
            return(-1);
        }
        return 0;
}

int giveInfoMesg(mesgStruct * mesgInfo){// controller must free this memory
        int     result, length;
        /* The length is essentially the size of the structure minus sizeof(mtype) */
        length = sizeof(mesgStruct) - sizeof(long);   
        if ( mesgInfo->number > 2665) LOGMESG( LOG_INFO , "Sending!!!  %d" ,(int) mesgInfo->number);
        if((result = msgsnd( controllerMainInfo.workers_qid, mesgInfo, length, 0)) == -1)
        {
                return(-1);
        }
        if ( mesgInfo->number > 2665) LOGMESG( LOG_INFO , "Sended!!!  %d" ,(int) mesgInfo->number);
        return(result);
}

int compressChunk(mesgStruct * mesgInfo){
    LOGMESG( LOG_INFO , "compressChunk of pointer data_decomp %p" , mesgInfo->data_decomp);
    size_t destLen = mesgInfo->size_comp;
    fileChunk * data_comp =(fileChunk *) mesgInfo->data_comp;
    data_comp->nextFileChunk = 0;
    char * data = ((char*)data_comp) + sizeof(fileChunk);
    mesgInfo->data_comp = (char*) data_comp;
    int ret, flush;
    size_t have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, mesgInfo->compression_lvl);
    size_t numberOfCompressedBytes = 0;

    if (ret != Z_OK)
        return ret;
    
    /* compress until end of file */
    do
    {
        size_t bytesLeft = mesgInfo->size_decomp;
        strm.avail_in = bytesLeft; 
        flush = Z_FINISH;
        strm.next_in = mesgInfo->data_decomp;
        do
        {
            if ( destLen - numberOfCompressedBytes < 0){
                LOGMESG( LOG_ERROR, "Too small destbuf size");
            }
            size_t current_out_buf_size = (destLen - numberOfCompressedBytes);
            strm.avail_out = current_out_buf_size ;
            //printf("bytesLeft = %d\n", (int) strm.avail_out);
            strm.next_out = data + numberOfCompressedBytes;            
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = current_out_buf_size - (size_t) strm.avail_out;
            //printf("Have = %d\n", (int) have);
            numberOfCompressedBytes += have;
            //if (fwrite(out, 1, have, dest)
            //!= have || ferror(dest)) { (void)deflateEnd(&strm);return Z_ERRNO; }
        }
        while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    }
    while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    mesgInfo->size_comp = numberOfCompressedBytes+sizeof(fileChunk);
    data_comp->size_of_data = numberOfCompressedBytes +sizeof(fileChunk);
    return 0;
}
     
size_t addFileToFL(int fd,char* nextFileName , size_t* FLoffset  ){
    LOGMESG(LOG_INFO, "addFileToFL addDirToFL , offset = %lld", ( long long) *FLoffset);

    
    struct sembuf sops[2];
        sem_change_ptr(sops, S_ADD_FILE_TO_FL , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
        sem_change_ptr(sops+1, S_ADD_FILE_TO_FL , 1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
    semop( controllerMainInfo.sem , sops, 2);
    
    struct stat stbuf;
    fDescription *fDesc = (fDescription*) malloc( sizeof(fDescription) + strlen(nextFileName)+1 );

    memcpy( (char*)fDesc + sizeof(fDescription) , nextFileName,  strlen(nextFileName)+1);
    if (fstat( fd, &stbuf) == -1){
    }
    fDesc->is_dir = 0;
    fDesc->st_mode = stbuf.st_mode;
    fDesc->st_uid = stbuf.st_uid;
    fDesc->st_gid = stbuf.st_gid;
    fDesc->st_size = stbuf.st_size;
    //fDesc->st_atime = stbuf.st_atime;
    //fDesc->st_mtime = stbuf.st_mtime;
    //fDesc->st_ctime = stbuf.st_ctime;
    size_t offset = writeToOut( (char*) fDesc , sizeof(fDescription)+strlen(nextFileName)+1);
    free(fDesc);
    fileList *FL =  ( fileList* )(controllerMainInfo.mmap_start + *FLoffset);
    addOffsetToFL( FL, FLoffset, offset);
    //FL->fDescriptionOffset[FL->count] = offset; // todo
    //FL->count = FL->count + 1;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    sem_change_ptr(sops, S_ADD_FILE_TO_FL , -1,SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
    semop( controllerMainInfo.sem , sops, 1);
    return offset;
}


size_t addDirToFL(int fd, char* nextDirName , size_t* FLoffset ){
    LOGMESG(LOG_INFO, "addDirToFL , offset = %lld", ( long long) *FLoffset);
    struct sembuf sops[2];
        sem_change_ptr(sops, S_ADD_FILE_TO_FL , 0, 0); //allow for only one process to be in the TAKE_NEXT_FILE function
        sem_change_ptr(sops+1, S_ADD_FILE_TO_FL , 1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
    semop( controllerMainInfo.sem , sops, 2);
    
    fileList FL_2; 
    FL_2.count = 0;
    FL_2.nextFLOffset = 0;
    size_t offset_to_fl_2;
    offset_to_fl_2 = writeToOut( (char*) &FL_2, sizeof(fileList));
    fDescription *fDesc = (fDescription*) malloc( sizeof(fDescription) + strlen(nextDirName)+1 );
    fDesc->is_dir = 1;
    fDesc->firstFileChunkOffset = offset_to_fl_2;
    memcpy( ((char*)fDesc) + sizeof(fDescription) , nextDirName,  strlen(nextDirName)+1);
    size_t offset = writeToOut( (char*) fDesc , sizeof(fDescription)+strlen(nextDirName)+1);
    free(fDesc);
    fileList *FL =  ( fileList* )(controllerMainInfo.mmap_start + *FLoffset);
    addOffsetToFL( FL, FLoffset, offset);
    //FL->fDescriptionOffset[FL->count] = offset; // todo
    //FL->count = FL->count + 1;////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    sem_change_ptr(sops, S_ADD_FILE_TO_FL , -1, SEM_UNDO | IPC_NOWAIT); //allow for only one process to be in the TAKE_NEXT_FILE function
    semop( controllerMainInfo.sem , sops, 1);
    return offset_to_fl_2;
}

int addOffsetToFL( fileList *FL ,size_t* FLoffset, size_t offset){
    size_t offset_to_fl_2;
    if ( FL->count == NUMDER_OF_FILES_IN_LIST ){
        fileList FL_2;
        FL_2.fDescriptionOffset[0] = offset;
        FL_2.count = 1;
        FL_2.nextFLOffset = 0;
        offset_to_fl_2 = writeToOut( (char*) &FL_2, sizeof(fileList));
        FL->nextFLOffset = offset_to_fl_2;
        FLoffset[0] = offset_to_fl_2;
    }else{
        FL->fDescriptionOffset[FL->count] = offset;
        FL->count = FL->count + 1;
    }
    return 0;
}

    
int dataRestore(char* ofname, char* filename){
    LOGMESG(LOG_INFO, "Data restore...");
    int fd;
    char* mmap_start;
    size_t length;
    size_t placeForNextChunkOffset;
    struct stat sb;
    int rc;
    size_t i;
    
    fd = open( filename , O_RDONLY);
    if ( fd == -1 ){
        LOGMESG(LOG_ERROR, "opening file");
    }
    fstat(fd, &sb);
    length = sb.st_size;
    mmap_start = (char*) mmap(NULL, length, PROT_READ ,
                    MAP_PRIVATE, fd, 0);
    if ( mmap_start == NULL){
        LOGMESG(LOG_ERROR ,"Error while creating mmap");
    }
    controllerMainInfo.mmap_start = mmap_start;
    fileList * FL = (fileList *) mmap_start;
    LOGMESG(LOG_INFO, "fileList files count = %d", (int)FL->count );
    
    
    struct stat st = {0};
    
    if (stat(ofname, &st) == -1) {
        rc = mkdir(ofname, 0777);
        if ( rc != 0 ) {
            LOGMESG(LOG_ERROR ,"Creating directory error");
            perror("");
            return -1;
        }
        rc = chdir(ofname);
        restoreFileList(FL);
        chdir("..");
    }else{
        fprintf(stderr, "Folder recovery exists\n");
        return -1;
    }
    return 0;
}
int restoreFileList(fileList *FL){
    char* mmap_start = controllerMainInfo.mmap_start;
    int i;
    LOGMESG(LOG_INFO, "fileList files count = %d", (int)FL->count );
    for( i = 0; i< FL->count; i++){
        restoreFile(FL->fDescriptionOffset[i]);
    }
    if ( FL->count == NUMDER_OF_FILES_IN_LIST && FL->nextFLOffset != 0  ){
        LOGMESG(LOG_INFO, "restoreFileList RESTORING ADDITIONAL FL, offset = %lld ", ( long long) FL->nextFLOffset );
        fileList *FLNEXT = (fileList*) (mmap_start + FL->nextFLOffset);
        restoreFileList(FLNEXT);
    }else{
    chdir("..");
    }
    return 0;
}

int restoreFile(  size_t offset_to_file ){
    int rc;
    char* mmap_start = controllerMainInfo.mmap_start;
    fDescription * f_description = (fDescription *) (mmap_start + offset_to_file);
    LOGMESG(LOG_INFO, "Restoring file... %s", ((char*)&f_description[1]) );
    if ( f_description->is_dir == 1){
        mkdir((char*)&f_description[1], 0777);
        chdir((char*)&f_description[1]);
        fileList * FL = (fileList * ) (mmap_start + f_description->firstFileChunkOffset);
        restoreFileList(FL);
        
        return 0;
    } 
    // tested component
    if ( S_ISLNK(f_description->st_mode) ){
        rc = symlink( mmap_start+f_description->firstFileChunkOffset , (char*) &f_description[1]);
        return 0;
    }
    
    int fd_out = open( (char*)&f_description[1] , O_CREAT | O_RDWR, f_description->st_mode);
    fileChunk * file_chunk = (fileChunk *) (mmap_start + f_description->firstFileChunkOffset);
    if ( f_description->st_size >0){
        while( 1 )
        {
            mesgStruct mesg;
            mesg.data_comp = (char*) file_chunk;
            LOGMESG(LOG_INFO, "restoreFile restoreChunk");
            if (restoreChunk(&mesg) == -1){
                LOGMESG(LOG_ERROR, "Error while restoring CHUNK...");
            }
            LOGMESG(LOG_INFO, "Restoring numberOfRestoredBytes = %d", (int) mesg.size_decomp);
            write( fd_out , mesg.data_decomp, mesg.size_decomp);
            free(mesg.data_decomp);
            if ( file_chunk->nextFileChunk == 0){
                LOGMESG(LOG_INFO, "Restoring of this file finished");
                break;
            }else{
                LOGMESG(LOG_INFO, "Restoring next chunk");
                file_chunk = (fileChunk *) (mmap_start + file_chunk->nextFileChunk);
            }
        }
    }
    close( fd_out);
}

int restoreChunk( mesgStruct* mesg){
    LOGMESG(LOG_INFO, "restoreChunk mesg = %p, mesg->data_comp = %p", mesg,mesg->data_comp);
    fileChunk * file_chunk = (fileChunk *)mesg->data_comp;
    char * data_decomp = (char*) malloc( CHUNK +1 ); // need to be free from outside
    if ( data_decomp == NULL){
        LOGMESG(LOG_ERROR, "restoreChunk, malloc");
        return -1;
    }
    char * data_comp = (char*) &file_chunk[1];
    size_t numberOfCompressedBytes = file_chunk->size_of_data - sizeof(fileChunk);
    size_t numberOfInflatedBytes = 0;
    mesg->data_decomp = data_decomp;
    LOGMESG(LOG_INFO, "Restoring numberOfCompressedBytes = %d", (int) numberOfCompressedBytes);
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = numberOfCompressedBytes;
        strm.next_in = data_comp;
        if (strm.avail_in == 0)
            break;
        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK-numberOfInflatedBytes + 1;
            strm.next_out = data_decomp+numberOfInflatedBytes;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                LOGMESG(LOG_ERROR, "restoreChunk Restoring Error");
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                LOGMESG(LOG_ERROR, "restoreChunk Restoring Error");
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - numberOfInflatedBytes + 1- strm.avail_out;
            LOGMESG(LOG_INFO, "Restoring have = %d", (int) have);
            numberOfInflatedBytes = numberOfInflatedBytes + have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    mesg->size_decomp = numberOfInflatedBytes;
    return ret == Z_STREAM_END ? 0 : -1;
}


