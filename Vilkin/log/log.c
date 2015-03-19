#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "log.h"

log_info_t * log_init(const char * file, int max_log_prior)
{
    log_info_t * info = (log_info_t*)malloc(sizeof(log_info_t));
    info->file_name = fopen(file, "w");
    info->max_prior = max_log_prior;
    info->max_size_log = 1000;
    info->start_time = clock();
    return info;
}

void get_time(log_info_t * log_info, char * str_dest)
{
    time_t curr_time;
    char * buf;
    int time_len = 0;
    time(&curr_time);
    time_len = strlen(ctime(&curr_time));
    buf = (char*) malloc(time_len + 1);
    strncpy(buf, ctime(&curr_time), time_len-1);
    sprintf(str_dest, "[%s - %.4lf]", buf, (double)(clock() - log_info->start_time)/CLOCKS_PER_SEC);
    free(buf);
}

int _log(const char * file, const char * func, int line, log_info_t * log_info, int prior, char * fmt, ...)
{
    va_list arg;
    char * res = (char*)malloc(log_info->max_size_log);
    char * str = (char*)malloc(log_info->max_size_log);
    if (prior <= log_info->max_prior)
    {
        get_time(log_info, res);
        sprintf(res, "%s [%s] Function: '%s' [%s:%d]  ", res, log_prior_name[prior], func, file, line);
        va_start(arg, fmt);
        vsprintf(str, fmt, arg);
        va_end(arg);
        strcat(res, str);
        fprintf(log_info->file_name, "%s\n", res);
    }
    free(str);
    free(res);
    return 0;
}

void log_exit(log_info_t * log_info)
{
    close(log_info->file_name);
    free(log_info);
}

