//
//  logger.h
//  log
//
//  Created by Daniil Sargin on 18/03/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#ifndef __log__logger__
#define __log__logger__

struct log_context
{
    FILE *fp;
    int prior;
    int flOpen;
};

struct log_context initlog(int prior, const char* flname, const char* mode);
int nlog(int prior, struct log_context* logc, const char *fmt, ...);
void deinitlog(struct log_context* logC);

#endif /* defined(__log__logger__) */
