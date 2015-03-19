//
//  logger.c
//  log
//
//  Created by Daniil Sargin on 18/03/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#include <stdio.h>
#include "logger.h"
#include <stdarg.h>
#include <sys/time.h>

struct log_context initlog(int prior, const char* flname, const char* mode)
{
    struct log_context result;
    result.prior = prior;
    result.fp = fopen(flname, mode);
    result.flOpen = 1;
    return result;
}

void deinitlog(struct log_context* logC)
{
    fclose(logC->fp);
    logC->flOpen = 0;
}

int nlog(int prior, struct log_context* logc, const char *fmt, ...)
{
    if (logc->flOpen == 0) {
        printf("nlog: no file opened \n");
        return 0;
    }
    if (prior > logc->prior) {
        return 0;
    }
    
    time_t t = time(NULL);
    struct tm* aTm = localtime(&t);
    
    //milliseconds
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    
    fprintf(logc->fp, "-> %04d/%02d/%02d %02d:%02d:%02d:%03d ", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, curtime.tv_usec/1000);
    
    va_list list;
    int done;
    va_start(list, fmt);
    done = vfprintf(logc->fp, fmt, list);
    fprintf(logc->fp, "\n");
    fflush(logc->fp);
    va_end(list);
    return done;
}