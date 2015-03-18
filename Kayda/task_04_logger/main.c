
#include <stdio.h>
#include <pthread.h>
#include "logger.h"


#define THR_NUM 4
#define THR_LOGS 2048

void *loggist(void *arg)
{
	int i, n = *(int*) arg;
	for (i = 0; i < THR_LOGS; i++)
		Log(1, "loggist #%02d: %d\n", n, i);
	pthread_exit(NULL);
}


int main()
{
	int i;
	pthread_t thr[THR_NUM];
	int dat[THR_NUM];

	initLog(2, "log");

	Log(0, "Log #%d\n", 0);
	Log(1, "Log #%d\n", 1);
	Log(2, "Log #%d\n", 2);
	Log(3, "Log #%d\n", 3);

	for (i = 0; i < THR_NUM; i++){
		dat[i] = i;
		pthread_create(&thr[i], NULL, &loggist, &dat[i]);
	}

	for (i = 0; i < THR_NUM; i++)
		pthread_join(thr[i], NULL);

	exitLog();

	return 0;
}

