
#ifndef LOGGER_H
#define LOGGER_H

#define CHECK_LOG_LEVEL( log_level )				\
	do {											\
		if( !isLogLevelCorrect(log_level) )			\
			return;									\
	} while(0)

typedef enum logLevel {
	INFO = 0,
	DEBUG,
	WARNING,
	ERROR,
	FATAL
} loglvl_t;

int initLog(loglvl_t cur_log_level, char *filename);

void LOG(loglvl_t log_level, char *fmt, ...);

int getCurLogLevel();

void setCurLogLevel(loglvl_t new_cur_log_level);

char *getLogLevelName(loglvl_t log_level);

int isLogLevelCorrect(loglvl_t log_level);

int closeLogFile();

#endif