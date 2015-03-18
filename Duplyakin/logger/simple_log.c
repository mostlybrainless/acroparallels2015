#include"simple_log.h"
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<pthread.h>

/* Global variables  */

/* Mutex (mutual exclusion) lock for critical operations*/
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
/* File where we write */
FILE * flog = NULL;
/* Privilege level */
int def_priv=0;

#define NPRI 3
#define BUFSIZE 300
/* Table-known priorities */
const char * priorities[NPRI]=
{
    "FATAL",
    "ERROR",
    "WARNING"
};

int Log_init(int priority, const char * file_name)
{
    pthread_mutex_lock(&log_mutex);
    def_priv=priority;
    if (flog)
	fclose(flog);
    flog=fopen(file_name,"a");
    pthread_mutex_unlock(&log_mutex);
    if (flog == NULL)
	return -1;
    else
	return 0;
}

void Log(int priority, const char * fmt,...)
{
    char buf[BUFSIZE];
    size_t l;
    time_t t;
    va_list v;

    if (priority < 0 || priority > def_priv || flog == NULL)
	return;
    /* Block out to give time to all records were consistently*/
    pthread_mutex_lock(&log_mutex);
    time(&t);
    ctime_r(&t,buf);
    l=strlen(buf)-1;
    if (priority >= NPRI)
	snprintf(buf+l,BUFSIZE-l-1,": PRIORITY: %d: ",priority);
    else
	snprintf(buf+l,BUFSIZE-l-1,": %s: ",priorities[priority]);

    fprintf(flog,"%s",buf);
    va_start(v,fmt);
    vfprintf(flog,fmt,v);
    va_end(v);
    pthread_mutex_unlock(&log_mutex);
}

void Log_close(void)
{
    fclose(flog);
    flog=NULL;
}
