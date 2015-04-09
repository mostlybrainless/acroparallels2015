#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define SIZE_OF_NAME 256

typedef enum _my_log_priority
{
	CRITICAL_ERROR,
	ERROR,
	WARNING,
	MESSAGE
}my_log_priority;

int my_log_init(my_log_priority priority, char* log_file_name);
void my_log(my_log_priority log_priority, char* format_str, ...);
void my_log_end();
