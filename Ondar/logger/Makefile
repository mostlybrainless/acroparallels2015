# compiler is gcc
CC=gcc
# some flags for gcc
CFLAGS=-c

all: example

example: example.o logger.o
	$(CC) example.o logger.o -o example

example.o: example.c
	$(CC) $(CFLAGS) example.c

logger.o: logger.c
	$(CC) $(CFLAGS) logger.c

clean:
	rm -rf *.o example log_file.txt
