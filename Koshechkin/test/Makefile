PROGPATH=..
INCLUDES=$(PROGPATH)/include
CC = g++

all: test_log run

test_log: $(INCLUDES)/log.h $(PROGPATH)/log/log.c test_log.c
	$(CC) -o test_log_out test_log.c $(PROGPATH)/log/log.c -I$(INCLUDES)

run:
	./test_log_out

clean:
	rm test_log_out*