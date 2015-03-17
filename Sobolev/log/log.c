#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>

#include "log.h"


struct log_struct logInfo = {-1, -1};

void initLog(int cur_log_level, char* filename) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND);
	printf("%d\n", fd);
	logInfo.destination = fd;
	logInfo.level = cur_log_level;
	char buffer[LOG_MAX_LENGTH];
	time_t t = time(NULL);
	time(&logInfo.start_time);
        struct tm * time = localtime(&t);
        int beg_len = sprintf(buffer, "\nBeginning of new log dated by %04d/%02d/%02d %02d:%02d:%02d\n",
                        time->tm_year+1900, time->tm_mon+1, time->tm_mday,
                        time->tm_hour, time->tm_min, time->tm_sec);
	write(logInfo.destination, buffer, beg_len);
	return;
}


void Log(int priority, char * format, ...) {
	va_list arg_list;
	va_start(arg_list, format);
	if (priority < logInfo.level)
		return;
	time_t current_time;
	time(&current_time);
	char buffer[LOG_MAX_LENGTH];
	double seconds = difftime(current_time, logInfo.start_time);
	int time_len = sprintf(buffer, "Time from beginning %.fms: ", seconds);
	int mess_len  = vsprintf(buffer + time_len, format, arg_list);
	write(logInfo.destination, buffer, mess_len + time_len);
	perror(buffer);
	va_end(arg_list);
	return;
}


void finishLog() {
	close(logInfo.destination);
}

