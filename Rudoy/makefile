CC=gcc
CFLAGS=-c -Wall
INC=-I./src/include/ -I./src/zlib-1.2.8
LDFLAGS=-lz -lrt

test_map: test_map.o log.o mapping.o archiver.o
	$(CC) test_map.o log.o mapping.o archiver.o  -o test_map  $(LDFLAGS)

test_log.o: test/test_log.c
	$(CC) $(INC) $(CFLAGS) test/test_log.c

test_map.o: test/test_map.c
	$(CC) $(INC) $(CFLAGS) test/test_map.c

archiver.o: src/archiver.c
	$(CC) $(INC) $(CFLAGS) src/archiver.c

mapping.o: src/mapping.c
	$(CC) $(INC) $(CFLAGS) src/mapping.c

log.o: src/log/log.c
	$(CC) $(INC) $(CFLAGS) src/log/log.c
 

