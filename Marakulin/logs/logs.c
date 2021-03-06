#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "../include/logs.h"

#define SIZE_OF_LOGBUF 300

static Logs logs;

void logsCreate(LogLevel curLogLevel, const char* fileName)
{
    if (curLogLevel > TRACE)
    {
        perror("ErrorLogging: LogLevel is invalid");
        exit(1);
    }
    logs._out = fopen(fileName, "w");
    if (logs._out == NULL)
    {
        perror("ErrorLogging: Can't open file for writing");
        exit(1);
    }
    logs._buffer = (char*)calloc(SIZE_OF_LOGBUF, sizeof(char));
    if (logs._buffer == NULL)
    {
        perror("ErrorLogging: Can't allocate memory");
        exit(1);
    }
    logs._curLogLevel = curLogLevel;
}

void releaseLogs()
{
    free(logs._buffer);
    free(logs._out);
}

void get(const char* func, const char* file, int line, LogLevel logL, const char* format, ...)
{
    if (logL > logs._curLogLevel)
        return;
    va_list vl;
    va_start(vl, format);
    time_t t = time(0);
    struct tm * now = localtime( & t );
    snprintf(logs._buffer, SIZE_OF_LOGBUF, "%d-%02d-%02d %02d:%02d:%02d '%s' %s() (%s:%d)\n",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday, 
             now->tm_hour, now->tm_min, now->tm_sec,
             format,
             func, file, line);
    vfprintf(logs._out, logs._buffer, vl);
    fflush(logs._out);
    va_end(vl);
}
