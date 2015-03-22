#include "my_log.h"

my_log_priority global_log_priority;
FILE* log_file_desc = NULL;

int my_log_init(my_log_priority priority, char* log_file_name)
{
	global_log_priority = priority;
	char name[SIZE_OF_NAME];
	strcpy_s(name, SIZE_OF_NAME, log_file_name == NULL ? "log.txt" : log_file_name);
	fopen_s(&log_file_desc, name, "w");
	if (log_file_desc == NULL) return 0;
	else return 1;
}

void my_log(my_log_priority log_priority, char* format_str, ...)
{
	if (log_priority <= global_log_priority)
	{
		time_t cur_time = time(NULL);
		struct tm aTm;
		localtime_s(&aTm, &cur_time);
		fprintf(log_file_desc, "%04d/%02d/%02d %02d:%02d:%02d --- ", aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday, aTm.tm_hour, aTm.tm_min, aTm.tm_sec);
		va_list arg_list;
		va_start(arg_list, format_str);
		vfprintf(log_file_desc, format_str, arg_list);
		fprintf(log_file_desc, "\n");
		va_end(arg_list);
	}
}

void my_log_end()
{
	fclose(log_file_desc);
}