

#ifndef ARCHIVER_H
#define ARCHIVER_H


#define MAGIC_HEADER 0x18DEAD04U

#define ERROR(...) do{ 					\
   		Log(LOG_ERROR, __VA_ARGS__); 	\
		fprintf(stderr, __VA_ARGS__); 	\
	} while(0)


struct arch_state{
	enum arch_action{
		ARCH_CREATE,
		ARCH_EXTRACT
	} action;

	uint8_t 	block_size_id;

	char* 		finname;
	void* 		finmap;
	uint64_t 	finsize;
	uint16_t 	finmode;

	char* 		foutname;
	int 		foutdesc;
	uint64_t 	foutsize;
};


int archMain(struct arch_state st);


#endif //ARCHIVER_H

