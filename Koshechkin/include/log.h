#ifndef _ARCHIVER_LOG_
#define _ARCHIVER_LOG_

/**
 * There will be description soon
 *
 *
 */

enum LOG_LVL
{
    ERROR = 0,  // Other runtime errors or unexpected conditions
    WARNING,    // Not critical errors, but they can lead to fatal ones
    INFO,       // Interesting runtime events (startup/shutdown)
    DEBUG,      // Detailed information on the flow through the system
    TRACE       // More detailed information
};

static char LOG_LVL_STR[5][8] = {"ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "TRACE  "};

int log_init (LOG_LVL log_cur_level, const char * log_filename);

int log_write(LOG_LVL log_level, const char* format, ...);

int log_close();

#endif // _ARCHIVER_LOG_