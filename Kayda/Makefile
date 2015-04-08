
CC=gcc
CFLAGS=-Iinclude -pthread

archiver: archiver.o lz4.o logger.o xxhash.o
	gcc $(CFLAGS) -o $@ $^

lz4.o: lib/lz4.c include/lz4.h
	gcc -c $(CFLAGS) -o $@ $<

logger.o: lib/logger.c include/logger.h
	gcc -c $(CFLAGS) -o $@ $<

xxhash.o: lib/xxhash.c include/xxhash.h
	gcc -c $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o archiver

