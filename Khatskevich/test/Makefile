PROGPATH=..
INCLUDES=$(PROGPATH)/include
TOOLSPATH=$(PROGPATH)/tools

all: log_test log_test_proc zpipe_test


log_test: $(INCLUDES)/log.h $(PROGPATH)/log/log.c log_test.c
	$(CC) -o log_test_out log_test.c $(PROGPATH)/log/log.c -pthread -I$(INCLUDES)
	./log_test_out
# may be it is better writer to be lunching in the separate process? 
# when main prog crashes, it will finished log.
log_test_proc: $(INCLUDES)/log.h $(PROGPATH)/log/log.c log_test.c
	$(CC) -o log_test_out log_test.c $(PROGPATH)/log/logProc.c -pthread -I$(INCLUDES)
zpipe_test: 
	$(CC) -o zpipe zpipe_test.c $(TOOLSPATH)/zlib/libz.a -I$(INCLUDES)
	./zpipe zpipe_file_for_deflate zpipe_file_for_inflate_out
	./zpipe -d <zpipe_file_for_inflate_out >zpipe_inflated_file_out
clean:
	rm log_test_out* zpipe *out
