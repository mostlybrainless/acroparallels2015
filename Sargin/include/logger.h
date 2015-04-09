//
//  logger.h
//  log
//
//  Created by Daniil Sargin on 18/03/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#ifndef __log__logger__
#define __log__logger__

#include <stdio.h>
typedef enum {
    LOG_FATAL = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} LOG_LEVEL;

typedef struct log_context_s {
    FILE *fp;
    LOG_LEVEL prior;
    int flOpen;
} log_context;

log_context initlog(LOG_LEVEL prior, const char * flname, const char * mode);
int nlog(LOG_LEVEL prior, log_context * logc, const char * fmt, ...);
void deinitlog(log_context * logC);

#endif /* defined(__log__logger__) */
