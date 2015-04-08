#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>    
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdlib.h>


#pragma once


#define ALOGA( ... ) LOG ( A , __VA_ARGS__)
#define ALOGD( ... ) LOG ( D , __VA_ARGS__)
#define ALOGW( ... ) LOG ( W , __VA_ARGS__)
#define ALOGE( ... ) LOG ( E , __VA_ARGS__)
#define ALOGF( ... ) LOG ( F , __VA_ARGS__)


/* Warning: SIZE_OF_SHARED_BUFFER must not be 1 !*/


#define SIZE_OF_SHARED_BUFFER 1024
#define MAX_LEN_OF_LOG 256
#define SHAREDMEM_FILENAME "log.h"
#define MIN(a,b) (((a) < (b) ) ? (a) : (b) )


enum level
{
	A,		//all
	D,		//debug_info
	W,		//warnings
	E,		//errors
	F,		//fatal_errors
	N		//without logs
};

typedef struct
{
	int begin;
	int end;
} buffer_info;


typedef struct
{
	buffer_info sh_buf_for_print;
	int last_write;
	char sh[SIZE_OF_SHARED_BUFFER][MAX_LEN_OF_LOG];
} shared_data_log;


extern shared_data_log* gSh_data;

pthread_mutex_t gMutex_attr;
int gLevel;
buffer_info gbuffer_info;
pid_t pid;                                // pid of buffer-handler proces
int fd;


void child_handler();
void kill_dump_proc();
void dump_log();
int init_log( int level, char * filename);
int transmit_buffer( int level, char *format, va_list ap );
void LOG( int level, char *format, ... );
shared_data_log* prepare_shm();
shared_data_log* take_shm_ch();
void prepare_for_signals();
char *level_info( int level);
void write_to_file();
