#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define YEAR_CONST 1900
#define MONTH_CONST 1
#define MAX_MESSAGE_LENGTH 512
#define MIN_MESSAGE_LEVEL 1

enum LEVEL {OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

char* getLevelName(int lvl) {
   switch (lvl) {
      case OFF: return "OFF";
      case FATAL: return "FATAL";
      case ERROR: return "ERROR";
      case WARN: return "WARN";
      case INFO: return "INFO";
      case DEBUG: return "DEBUG";
      case TRACE: return "TRACE";
      case ALL: return "ALL";
   }
}

typedef struct logger
{
	int cur_log_level;
	FILE* logfile;
} logger;

logger* log_init(int log_level, char* file_path) {
	logger* newlog = (logger*)malloc(sizeof(logger));
	if (newlog == NULL) {
		fprintf(stderr, "Failed to allocate memory for the log\n");
		return NULL;
	}
	newlog->cur_log_level = log_level;
	newlog->logfile = fopen(file_path, "w");
	if (newlog->logfile == NULL) {
		fprintf(stderr, "Failed to open a file %s for writing\n", file_path);
		return NULL;
	}
	return newlog;
}

int log_close(logger* log) {
	if (fclose(log->logfile) == EOF) {        //If the stream is successfully closed, a zero value is returned. On failure, EOF is returned.
		fprintf(stderr, "Can't close the log file");
		return -1;
	}
	free(log);
	return 1;
}

int log_msg(logger* log, int prior, char* message, ...) {
	if (prior > log->cur_log_level || strlen(message) > MAX_MESSAGE_LENGTH || prior < MIN_MESSAGE_LEVEL)
		return 0;
	else {
		time_t t = time(NULL);
		if (t == -1) {
			fprintf(stderr, "Can't get the current calendar time");
			return -1;
		}
		struct tm tm = *localtime(&t);
		if (fprintf(log->logfile, "%d:%02d:%d %d:%d:%d %s %s\n", YEAR_CONST + tm.tm_year, MONTH_CONST + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getLevelName(prior), message) < 0) {
			fprintf(stderr, "Can't write to log file");
			return -1;
		}
	}
	return 1;
}

void set_log_level(logger* log, int new_log_level) {
	log->cur_log_level = new_log_level;
}

int main(int argc, char *argv[]) {
	logger* log = log_init(3, "logfile.txt");
	log_msg(log, 3, "My first report to the logfile. Hoping this will pass.");
	log_msg(log, 5, "This shouldn't pass");
	log_msg(log, 1, "I don't think that there should be very-very-very long messages in the logfile. At least we can regulate the length of them. I'll try to make this message so long that it won't pass, but I really don't know what to write. So instead of writing some stupid things I will just repeat the message twice. I don't think that there should be very-very-very long messages in the logfile. I'll try to make this message so long that it won't pass, but I really don't know what to write. So instead of writing some stupid things I will just repeat the message once again. I don't think that there should be very-very-very long messages in the logfile. I'll try to make this message so long that it won't pass, and looks like I did it.");
	log_msg(log, -234, "it's strange to see messages with negative level numbers in the log");
	set_log_level(log, 0);
	log_msg(log, 1, "I closed the writing to logfile, so this message won't pass");
	log_close(log);
	return 0;
}