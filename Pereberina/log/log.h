//
//  log.h
//
//
//  Created by Anastasia Pereberina on 13.03.15.
//
//

#ifndef ____log__
#define ____log__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#endif /* defined(____log__) */
#define log_exit(str) { perror(str); if (message) free(message); if(log) free(log); deinit_log(); exit(errno); }

enum LOG_LEVEL {
    FATAL = 0,
    ERROR,
    WARNING,
    DEBUG,
    INFO,
    
    __MAX_LEVEL
};

static unsigned int CUR_LOG_LEVEL = 0;
static int file = -1;

int init_log(unsigned int cur_log_level, char *filename) {
    if (cur_log_level >= __MAX_LEVEL) {
        perror("Forbidden log level");
        return 1;
    }
    CUR_LOG_LEVEL = cur_log_level;
    if ((file = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0) {
       	perror("Can't open file");
        exit(errno);
    }
    return 0;
}

void deinit_log(void) {
    CUR_LOG_LEVEL = 0;
    close(file);
    file = -1;
}

int _log(unsigned int prior, const char * format, ...) {
    if (file < 0) {
        perror("Can't open log file");
        return 1;
    }
    if (prior > CUR_LOG_LEVEL)
        return 0;
    
    int tm_size, lvl_size, mess_size;
    time_t *tp;
    va_list marker;
    char *tm;
    char *message = NULL;
    char *log = NULL;
    
    if ((tp = (time_t *)malloc(sizeof(time_t))) == NULL)
        log_exit("Not enough memory");
    
    time(tp);
    tm = ctime(tp);
    free(tp);
    tm_size = strlen(tm);
    tm[tm_size-1] = '\t';
    
#define PRINT_LOG_LEVEL(lvl) #lvl
    char *PrintLogLevel[__MAX_LEVEL] = {
        PRINT_LOG_LEVEL(FATAL:\t),
        PRINT_LOG_LEVEL(ERROR:\t),
        PRINT_LOG_LEVEL(WARNING:\t),
        PRINT_LOG_LEVEL(DEBUG:\t),
        PRINT_LOG_LEVEL(INFO:\t)
    };
    
    lvl_size = strlen(PrintLogLevel[prior]);
    
    if ((message = (char *)malloc(sizeof(char)*strlen(format))) == NULL)
        log_exit("Not enough memory");
    
    va_start(marker, format);
    vasprintf(&message, format, marker);
    va_end(marker);
    mess_size = strlen(message);
    
    if ((log = (char *)malloc(sizeof(char)*(mess_size+tm_size+lvl_size))) == NULL)
        log_exit("Not enough memory");
    strcat(log, tm);
    strcat(log, PrintLogLevel[prior]);
    strcat(log, message);
    free(message);
    message = NULL;
#undef PRINT_LOG_LEVEL
    
    if (write(file, log, strlen(log)) < 0)
        log_exit("Writing failed");
    free(log);
    return 0;
}


