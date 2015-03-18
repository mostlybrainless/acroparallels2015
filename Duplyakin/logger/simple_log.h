#ifndef __SIMPLE_LOG_H__
#define __SIMPLE_LOG_H__

#define LOG_FATAL 0
#define LOG_ERROR 1
#define LOG_WARNING 2

int Log_init(int priority, const char * file_name);
void Log(int priority, const char * fmt, ...);
void Log_close(void);

#endif
