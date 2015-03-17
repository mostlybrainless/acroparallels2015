#include <iostream>

#include <stdlib.h>
#include <stdarg.h>

#include "logs.h"

#define SIZE_OF_LOGBUF 300

Logs::Logs(LogLevel curLogLevel, const char* fileName)
{
    if (curLogLevel > TRACE)
    {
        std::cerr << "ErrorLogging: LogLevel is invalid" << std::endl;
        exit(1);
    }
    _out = fopen(fileName, "a");
    if (_out == NULL)
    {
        std::cerr << "ErrorLogging: Can't open '" << fileName << "' for write" << std::endl;
        exit(1);
    }
    _buffer = (char*)calloc(SIZE_OF_LOGBUF, sizeof(char));
    if (_buffer == NULL)
    {
        std::cerr << "ErrorLogging: Can't allocate memory" << std::endl;
        exit(1);
    }
    _curLogLevel = curLogLevel;
}

Logs::~Logs()
{
    free(_buffer);
    fclose(_out);
}

void Logs::get(const char* func, const char* file, int line, LogLevel logL, const char* format, ...)
{
    if (logL > _curLogLevel)
        return;
    va_list vl;
    va_start(vl, format);
    time_t t = time(0);
    struct tm * now = localtime( & t );
    snprintf(_buffer, SIZE_OF_LOGBUF, "%d-%02d-%02d %02d:%02d:%02d '%s' %s() (%s:%d)\n",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday, 
             now->tm_hour, now->tm_min, now->tm_sec,
             format,
             func, file, line);
    vfprintf(_out, _buffer, vl);
    fflush(_out);
    va_end(vl);
}
