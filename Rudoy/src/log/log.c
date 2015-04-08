#include "defines.h"




/* Пока реализован только линейный буффер, причем конечного размера (SIZE_OF_SHARED_BUFFER), размер единичного лога так же пока ограничен (MAX_LEN_OF_LOG) */



shared_data_log* gSh_data = NULL;


char *level_info( int level)
{
	switch  ( level )
	{
		case A : return "all: ";
		case D : return "debug: ";
		case W : return "warning: ";
		case E : return "error: ";
		case F : return "fatal error: ";
	default : return NULL;
	}
}
	

int init_log( int level, char * filename)
{
	if (( level > F ) || ( level < A ) )
		return -1;

	if ( gSh_data != NULL )                       // if init_log has already done
		return -1;
	
	gLevel = level;
	gSh_data = prepare_shm();
	if ( gSh_data == NULL )
		return -1;
	
	pthread_mutex_init(&gMutex_attr, NULL);
	gSh_data->sh_buf_for_print.begin = 0;
	gSh_data->sh_buf_for_print.end = 0;
	gSh_data->last_write = 0;

	prepare_for_signals();

	fd = open( filename, O_CREAT|O_RDWR|O_TRUNC, 0777);
	if ( fd == -1 )
	{
		perror("Error in open");			//free shared memorhy
		printf("\n");
		return -1;
	}
	
	pid = fork();
	if ( pid == 0 )   					// child must always dump log
	{
		prctl(PR_SET_PDEATHSIG, SIGHUP); 		// for informing about parent's death
		gSh_data = take_shm_ch();
		if ( gSh_data == NULL )
		{	
			printf("cannot take shared memory\n");
			return -1;
		}
		while(1)
			pause();
		return -1;	
	}
	else if ( pid > 0 )         //parent
	{
	//	pause();                                    //waiting child preparation
		return 0;
	}
	else 
	{
		perror("Error in fork");
		return -1;
	}
}


void LOG( int level, char *format, ... )
{
	if ( ( gLevel > level ) || ( level > F ) || ( level < A ) || ( gSh_data == NULL ))
		return;
	va_list ap;
	va_start(ap, format);
	transmit_buffer( level, format, ap); 
	va_end(ap);
}


void transmit_buffer( int level, char *format, va_list ap)
{	
	pthread_mutex_lock( &gMutex_attr);
	while( gSh_data->sh_buf_for_print.end == gSh_data->sh_buf_for_print.begin - 1);		    //wait when buffer will be printed	
	int len = 0, num, index = gSh_data->last_write;
	char lev[15];
	strcpy( lev, level_info( level));
	num = strlen( lev);
	strncpy( gSh_data->sh[index], lev, num);
	len = vsnprintf( gSh_data->sh[index] + num, MAX_LEN_OF_LOG - num, format , ap);
	if ( len >= MAX_LEN_OF_LOG - num - 2   )              // 2 for '\0' and '\n'
	{
		*(gSh_data->sh[index] + MAX_LEN_OF_LOG - 2) = '\n';
		*(gSh_data->sh[index] + MAX_LEN_OF_LOG - 1) = '\0';
		//strncpy( gSh_data->sh[index] +  MAX_LEN_OF_LOG - MIN(MAX_LEN_OF_LOG , strlen( BUFFER_OVERFLOW_MSG) ) , BUFFER_OVERFLOW_MSG, MIN(MAX_LEN_OF_LOG , strlen( BUFFER_OVERFLOW_MSG) ));
	}
	else
	{
		*(gSh_data->sh[index] + len + num ) = '\n';
		*(gSh_data->sh[index] + len + num + 1) = '\0';
	}		
	gSh_data->last_write = (index + 1) % SIZE_OF_SHARED_BUFFER;
	gSh_data->sh_buf_for_print.end = gSh_data->last_write;
	kill( pid, SIGUSR1);									//inform buffer-handler proc	
	pthread_mutex_unlock( &gMutex_attr);
}


void dump_log()
{
	sigset_t   set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGHUP);
	sigprocmask(SIG_BLOCK, &set, NULL); 				//blocked SIGHUP( to no interrupt executing of SIGUSR1 handel) if we are late nothing will appear, because nothing is  changed
	write_to_file();
	sigdelset(&set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &set, NULL);                            // unblock SIGHUP  
}


void kill_dump_proc()
{
	int status;
	write_to_file();
	status = shm_unlink( SHAREDMEM_FILENAME);
	close( fd);
	if ( status == -1 )
	{
		perror( "Error in shm_unlink");
		exit(-1);
	}
	exit(0);
}

void write_to_file()
{
	int len, tmp_end;
	tmp_end = gSh_data->sh_buf_for_print.end;
	len = tmp_end - gSh_data->sh_buf_for_print.begin;
	if ( len < 0 )
		len = SIZE_OF_SHARED_BUFFER + len;
	int i = 0;
	for ( i = 0; i < len; i++ )
		write( fd, gSh_data->sh[(gSh_data->sh_buf_for_print.begin + i) % SIZE_OF_SHARED_BUFFER ], strlen( gSh_data->sh[(gSh_data->sh_buf_for_print.begin + i) % SIZE_OF_SHARED_BUFFER]));
	gSh_data->sh_buf_for_print.begin = tmp_end;
}

shared_data_log* prepare_shm()
{
	int fd, status;
	fd = shm_open(SHAREDMEM_FILENAME, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR);
	if ( fd == -1 )
	{
		perror( "Error in shm_open");
		return NULL;
	}
	status = ftruncate(fd, sizeof(shared_data_log));
	if ( status == - 1)
	{
		perror( "Error in ftruncate");
		status = shm_unlink( SHAREDMEM_FILENAME);
		if ( status == -1 )
		{
			perror( "Error in shm_unlink");
			return NULL;
		}
		return NULL;
	}
	shared_data_log* sdata = ( shared_data_log*) mmap(0, sizeof( shared_data_log), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return sdata;
}


shared_data_log* take_shm_ch()
{
	int fd;
	fd = shm_open(SHAREDMEM_FILENAME, O_RDWR, S_IRUSR|S_IWUSR);
	if ( fd == -1 )
	{
		perror( "Error in shm_open_child");
		return NULL;
	}
	shared_data_log* sdata = (shared_data_log*)mmap(0, sizeof(shared_data_log), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return sdata;
}


void prepare_for_signals()		// SIGHUP is sent then parent will die and SIGUSR1 is sent when something will appear in buffer
{
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = dump_log;
		sigset_t   set; 
		sigemptyset(&set);
		sigaddset(&set, SIGUSR1);
		sigaddset(&set, SIGHUP);
		act.sa_mask = set;
		sigaction( SIGUSR1, &act, 0);
		act.sa_handler = kill_dump_proc;
		sigaction( SIGHUP, &act, 0);
}
