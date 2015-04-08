#ifndef _ARCHIVATOR_
#define _ARCHIVATOR_

#include "log.h"
#include "data_presentation.h"
#include "sorted_mesg_stack.h"
#include "scaner.h"
#include "archivator.h"

#define CHUNK ( 256*1024 )
#define MAX_COMP_CHUNK_LEN ( CHUNK*2 + 1000 )

#define FORA_IN_CHUNKS 14
#define NUMBER_OF_WORKERS 10
#define NUMBER_OF_SCANERS 6
#define MAX_FILE_LENGTH 80000000000llu
#define TYPE_WORKER 1 
#define MAXIMAL_STACK_OF_DIRECTORIES 250
#define MAXIMAL_INPUT_FILES 1000 

#define MESG_SEQ_KEY 3
#define SEMAFORS_KEY 0
#define S_TAKE_NEXT_FILE        0
#define S_ADD_FILE_TO_FL        1
#define S_WRITE_TO_OUT          2 
#define S_PMALLOC               3 

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

int sem_change_ptr(struct sembuf *sops, short num , short value , short flag);//helps for semaphoring
//Ptr, number of semaphore, value to encrease, flags



#endif //_ARCHIVATOR_
