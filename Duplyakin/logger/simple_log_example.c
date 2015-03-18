#include"simple_log.h"
#include<pthread.h>
#include<stdlib.h>

/*
   Threads that will write in parallel in the log
   It is given a string that should be printed.
   Firstly it will wait for a second. Then,
   it will display the string and the number in the log. 30 times
   In this example there will be two threads working in this way
*/


void * logging_thread(void * arg)
{
    const char * msg=(const char *)arg;
    int i;
    sleep(1);
    for (i=0;i<30;i++)
	Log(LOG_ERROR,"%s:%d\n",msg,i);
    return NULL;
}

int main()
{
    pthread_t threads[2];
    void * res[2];
    Log_init(LOG_ERROR,"errors.log");
    Log(LOG_FATAL,"Cannot sleep, nightmares for %d hours\n",rand()%24);
    Log(LOG_ERROR,"Cannot HAHA for -1 hours\n");
    Log(LOG_WARNING,"Cannot BLABLA for -1 hours\n");
    /* Crating the first and the second line */
    pthread_create(&threads[0],NULL,logging_thread,"thread one");
    pthread_create(&threads[1],NULL,logging_thread,"thread two");
    /* waiting for their completion  */
    pthread_join(threads[0],&res[0]);
    pthread_join(threads[1],&res[1]);
    Log_close();
}
