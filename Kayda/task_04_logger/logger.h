

#ifndef LOGGER_H
#define LOGGER_H

#define Log(a, b, ...) logInternal(a, b, __FILE__, __LINE__, __func__, __VA_ARGS__)

int initLog(unsigned const logLevel, char *logName);
int exitLog();
void logInternal(unsigned level, char *msg, char *file, int line, const char *func, ...);

typedef enum  {
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_FLOW
} log_level_t;


#endif //LOGGER_H

