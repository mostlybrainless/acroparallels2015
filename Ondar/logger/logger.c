
/*
 * This is a logger.
 * Created by ondar07, 17/03/2015
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "logger.h"

static FILE *fp;
static loglvl_t curLogLevel = INFO;

// return 1, if initialization is successful
// else 0
int initLog(loglvl_t cur_log_level, char *filename) {

	CHECK_LOG_LEVEL(cur_log_level);

	fp = fopen(filename, "a+");
	curLogLevel = cur_log_level;

	if(!fp) {
		printf("Error with opening the file");
		return 0;
	} else {
		/* success */
		return 1;
	}
}

void LOG(loglvl_t log_level, char *fmt, ...) {
	time_t currentTime;
	struct tm *localTime;
	va_list ap;
	char *s;
	int d;
	void *p;

	//CHECK_LOG_LEVEL(log_level);

	// priority of this message is less than curLogLevel
	// so it won't be printed to the log_file
	if(log_level < curLogLevel)
		return;

	time(&currentTime);

	//fprintf(fp, "%s | ", ctime(&currentTime) );

	localTime = localtime(&currentTime);
	fprintf(fp,"[%d:%d:%d, %d.%d.%d] | ", 	localTime->tm_hour,
											localTime->tm_min,
											localTime->tm_sec,
											localTime->tm_mday,
											localTime->tm_mon + 1,
											localTime->tm_year + 1900);

	fprintf(fp, "[%s] | ", getLogLevelName(log_level));
	va_start(ap, fmt);

	vfprintf(fp, fmt, ap);

	va_end(ap);

	// for night guards of a code :)
	if(localTime->tm_hour >= 2 && localTime->tm_hour < 6)
		fprintf(fp, "\n\t You should go to bed, \n\t continue to conquer the world tomorrow :)");
	fputc('\n', fp);
}

char *getLogLevelName(loglvl_t log_level) {
	switch(log_level) {
		case INFO:
			return "INFO";
		case DEBUG:
			return "DEBUG";
		case WARNING:
			return "WARNING";
		case ERROR:
			return "ERROR";
		case FATAL:
			return "FATAL";
		default:
			printf("Such log level doesn't exist!\n");
			return NULL;
	}
}

int isLogLevelCorrect(loglvl_t log_level) {
	if(log_level < INFO || log_level > FATAL) {
		printf("Error with log level!\n");
		return 0;
	} else {
		return 1;
	}
}

int closeLogFile() {
	if(!fclose(fp)) {	// success
		return 1;
	}
	else {
		printf("Error with closing file");
		return 0;
	}
}

int getCurLogLevel() {
	return curLogLevel;
}

void setCurLogLevel(loglvl_t new_cur_log_level) {
	CHECK_LOG_LEVEL(new_cur_log_level);
	curLogLevel = new_cur_log_level;
}
