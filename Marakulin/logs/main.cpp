#define DEBUG_MODE
#include "./logs/logs.h"

int main()
{
    LOG_CREATE(INFO, "_debug");
    LOG(ERROR, "hello with number %d with string %s", 1, "My name is");
    LOG(WARNING, "hello 2");
    LOG(INFO, "hello 3");
    LOG(DEBUG, "hello 4");
    LOG(TRACE, "hello 5");
    return 0;
}
