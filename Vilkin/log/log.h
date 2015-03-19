#ifndef _LOG_H_
#define _LOG_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define log(...) _log(__FILE__, __func__, __LINE__, __VA_ARGS__)

enum LOG_PRIOR
{
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG
};

static char *log_prior_name[] = 
{
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

typedef struct
{
    FILE * file_name;
    int max_prior;
    int max_size_log;
    clock_t start_time;
} log_info_t;


log_info_t * log_init(const char * file, int max_log_prior);
void get_time(log_info_t * log_info, char * str_dest);
int _log(const char * file, const char * func, int line, log_info_t * log_info, int prior, char * fmt, ...);
void log_exit(log_info_t * log_info);

#endif // _LOG_H_

