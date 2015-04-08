#ifndef __LOGS__
#define __LOGS__

#include <stdio.h>

typedef enum {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE
} LogLevel;

typedef struct _Logs
{
	FILE* _out;
    const char* _fileName;
    LogLevel _curLogLevel;
    char* _buffer;
} Logs;

void logsCreate(LogLevel curLogLevel, const char* fileName);
void releaseLogs();
void get(const char* func, const char* file, int line, LogLevel logL, const char* format, ...);


#ifdef DEBUG_MODE

#define LOG_CREATE(curLogLevel, fileName) \
logsCreate(curLogLevel, fileName)

#define LOG_RELEASE() \
releaseLogs()

#define LOG(logL, format, ...) \
get(__FUNCTION__, __FILE__, __LINE__, logL, format, ## __VA_ARGS__)

#else

#define LOG_CREATE(curLogLevel, fileName)
#define LOG_RELEASE()
#define LOG(logL, format, ...) 

#endif


#endif //LOGS__
