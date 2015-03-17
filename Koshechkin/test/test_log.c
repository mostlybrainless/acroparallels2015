#include "log.h"


int main(int argc, char** argv)
{
	log_write(ERROR, "Error check %d", 1);
	log_init(WARNING, "mylog.log");
	log_write(ERROR, "Error check %d", 2);
	log_write(DEBUG, "Debug check %d", 3);
	log_write(WARNING, "Warning check %d", 4);
	log_write(INFO, "Info check %d", 5);
	log_write(ERROR, "Error check %d", 6);
	log_close();
	log_write(ERROR, "Error check %d", 6);
}