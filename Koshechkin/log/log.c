#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "log.h"

#define SUCCESS 1
#define FAILED 0

typedef struct
{
    int created;
    FILE* pFile;
    LOG_LVL cur_level;
} LOG_INFO;


LOG_INFO LOG = {.created = 0};

int log_init (LOG_LVL log_cur_level, const char * log_filename)
{
	if (LOG.created)
	{
		printf("LOG is already created\n");
		return FAILED;
	}
	if(!(LOG.pFile = fopen(log_filename, "w")) )
	{
		printf("Can't open %s\n", log_filename);
		return FAILED;
	}
	LOG.created = 1;
	LOG.cur_level = log_cur_level;
	log_write(DEBUG, "Start logging");
	return SUCCESS;
}


int log_write(LOG_LVL log_level, const char* format, ...)
{
	if (!LOG.created)
	{
		printf("LOG is not initialized.\n");
		return FAILED;
	}
	if (log_level > LOG.cur_level)
		return SUCCESS;

	va_list args;
	va_start (args, format);
	time_t t = time(NULL);
    tm* aTm = localtime(&t);
	fprintf (LOG.pFile, "%04d/%02d/%02d %02d:%02d:%02d:" ,aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	fprintf (LOG.pFile, " %07s: ", LOG_LVL_STR[log_level]);
	vfprintf (LOG.pFile, format, args);
	fprintf (LOG.pFile, "\n");
	va_end (args);
	return SUCCESS;
}

int log_close()
{
	LOG.created = 0;
	fclose(LOG.pFile);
	LOG.pFile = NULL;
	return SUCCESS;
}