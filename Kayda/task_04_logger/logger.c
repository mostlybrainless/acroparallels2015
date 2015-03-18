
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define BUF_SIZE 256
#define STR_SIZE 256

static unsigned loglvl;
static FILE *logfile = NULL;
static struct timeval logtime;
static char logbuffer[BUF_SIZE][STR_SIZE];
static unsigned firstindex = 0, lastindex = 0, commitindex = 0;
static pthread_t logthread;
static int logsem;

static char *log_level_str[] = {
	"ERROR",
	"Warn",
	"info",
	"flow"
};

/**
 * Worker for async write log on disk.
 * Use SVIPC semaphore to avoid busywaits.
 */
void *logWrite()
{
	struct sembuf sbuf;
	sbuf.sem_num = 0;
	sbuf.sem_op = -1;
	sbuf.sem_flg = 0;
	while (1){
		if ((semop(logsem, &sbuf, 1) == -1) && (errno == EIDRM || errno == EINVAL)){
			/* If we remove semaphore in main thread */
			while (firstindex != lastindex)
				fprintf(logfile, logbuffer[firstindex++ % BUF_SIZE]);
			pthread_exit(NULL);
			return;
		}
		fprintf(logfile, logbuffer[firstindex % BUF_SIZE]);
		firstindex++;
	}
}

/**
 * Lock-free message passing to ring buffer.
 * Use spinlocks for small waits.
 */
void logInternal(unsigned level, char *msg, char *file, int line, const char *func, ...)
{
	if (!logfile){
		fprintf(stderr, "Log system need init\n");
		return;
	}
	if (level <= loglvl){
		va_list ap;
		va_start(ap, func);
		struct timeval curtime;

		/* atomic for get place in queue */
		unsigned currentindex = __sync_fetch_and_add(&commitindex, 1);
		/* if ring buffer is full perform rescheduling (soft busy wait) */
		while (currentindex >= firstindex + BUF_SIZE)
			sched_yield();

		gettimeofday(&curtime, NULL);
		int sec, msec;
		sec = curtime.tv_sec - logtime.tv_sec;
		msec = (curtime.tv_usec - logtime.tv_usec)/1000;
		if (msec < 0){
			msec += 1000;
			sec--;
		}
		int n = snprintf(logbuffer[currentindex % BUF_SIZE], STR_SIZE,
				"%04ld.%03ld    [%s] %s:%u, in %s()\t", 
				sec, msec, log_level_str[level], file, line, func);
		vsnprintf(logbuffer[currentindex % BUF_SIZE] + n, STR_SIZE - n,  msg, ap);

		/** 
		 * Wait for slow threads with lower currentindex.
		 * Note: in busy states with number of logging threads > cores
		 * it will be better to use sched_yield() in loop.
		 */
		while (lastindex != currentindex)
			;
		lastindex++;
		struct sembuf sbuf;
		sbuf.sem_num = 0;
		sbuf.sem_op = +1;
		sbuf.sem_flg = 0;
		semop(logsem, &sbuf, 1);

		va_end(ap);
	}
}


int initLog(unsigned const logLevel, char *logName)
{
	loglvl = logLevel;
	logfile = fopen(logName, "w");
	if (!logfile){
		perror("initLog err:");
		return -1;
	}
	gettimeofday(&logtime, NULL);
	fprintf(logfile, "Log created at %s", ctime(&(logtime.tv_sec))); 
	logsem = semget(IPC_PRIVATE, 1, 0666);
	if (logsem == -1){
		fclose(logfile);
		perror("initLog semget:");
		return -1;
	}

	pthread_create(&logthread, NULL, &logWrite, NULL);

	return 0;
}


int exitLog()
{
	semctl(logsem, 0, IPC_RMID);
	pthread_join(logthread, NULL);
	fclose(logfile);
}

