#include <stdio.h>

enum LogLevel {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE
};

class Logs
{
private:
    FILE* _out;
    LogLevel _curLogLevel;
    char* _buffer;
public:
    Logs(LogLevel curLogLevel, const char* fileName);
    ~Logs();
    void get(const char* func, const char* file, int line, LogLevel logL, const char* format, ...);
};


#ifdef DEBUG_MODE

#define LOG_CREATE(curLogLevel, fileName) \
Logs log(curLogLevel, fileName)

#define LOG(logL, format, ...) \
log.get(__FUNCTION__, __FILE__, __LINE__, logL, format, ## __VA_ARGS__)

#else

#define LOG_CREATE(curLogLevel, fileName) 
#define LOG(logL, format, ...) 

#endif
