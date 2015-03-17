PROGPATH=..
INCLUDES=$(PROGPATH)/include

all: log_test log_test_proc


log_test: $(INCLUDES)/log.h $(PROGPATH)/log/log.c log_test.c
	$(CC) -o log_test_out log_test.c $(PROGPATH)/log/log.c -pthread -I$(INCLUDES)
	./log_test_out
# may be it is better writer to be lunching in the separate process? 
# when main prog crashes, it will finished log.
log_test_proc: $(INCLUDES)/log.h $(PROGPATH)/log/log.c log_test.c
	$(CC) -o log_test_out log_test.c $(PROGPATH)/log/logProc.c -pthread -I$(INCLUDES)
	./log_test_out
clean:
	rm log_test_out*
