

#ifndef LOGGER_H
#define LOGGER_H

#define Log(log_lvl, ...) logInternal(__FILE__, __LINE__, __func__, log_lvl, __VA_ARGS__)

int initLog(unsigned const logLevel, char *logName);
int exitLog();
void logInternal(char *file, int line, const char *func, unsigned level, char *msg, ...);

typedef enum  {
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_FLOW
} log_level_t;


#endif //LOGGER_H

