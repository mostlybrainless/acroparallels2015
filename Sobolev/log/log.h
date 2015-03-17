#ifndef MY_LOG
#define MY_LOG

#define LOG_MAX_LENGTH 1000

enum LOG_LEVELS {
	
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL	
};

struct log_struct {
	int destination;
	int level;	
	time_t start_time;
};

void initLog(int cur_log_level, char* filename);
void Log(int priority, char * format, ...);
void finishLog();

#endif
