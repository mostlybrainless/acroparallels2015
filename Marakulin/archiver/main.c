#include "main.h"

int main(int argc, char* argv[])
{
    LOG_CREATE(TRACE, "./debug.log");
    analyzeParams(argc, argv);
    LOG_RELEASE();
    return 0;
}
