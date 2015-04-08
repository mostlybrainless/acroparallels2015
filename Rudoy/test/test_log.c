#include "defines.h"


int main()
{
	int i = 10;
	double d = 2.24;
	char c = 'c';
	if ( init_log( D, "map.log") == -1)
	{
		shm_unlink( SHAREDMEM_FILENAME);
		return -1;
	}
	ALOGA( "ALL");
	ALOGD( "debug");
	sleep(4);
	ALOGW ( "warningwarningwarningwarningwarningwarningwarningwarningwarningwarningwarningwarningwarningwarning");
	ALOGE ( "errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror %d %f %c", i, d, c);
	ALOGF ( "fatal error\n");
	return 0;
}
