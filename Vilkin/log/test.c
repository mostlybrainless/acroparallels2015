#include <stdio.h>
#include "log.h"

int main()
{
    log_info_t * log_info = log_init("log.txt", WARN);
    log(log_info, FATAL, "Log record #%d", 1);
    log(log_info, ERROR, "Log record #%d", 2);
    log(log_info, WARN, "Log record #%d", 3);
    log(log_info, INFO, "Log record #%d", 4);
    log(log_info, DEBUG, "Log record #%d", 5);
    log_exit(log_info);
    return 0;
}
