CC=gcc
INCPATH=./include
LIBPATH=./archiver/libs
SOURCEPATH=./archiver

CFLAGS=-c -Wall
LD = -L./archiver/libs -lLogs -lz

all: arch

arch: main.o paramsAnalysis.o compressFile.o hashFunc.o
	cd ./logs && make
	$(CC) -I$(INCPATH) -L$(LIBPATH) -Wl,-R$(LIBPATH) main.o paramsAnalysis.o compressFile.o hashFunc.o -o arch -lLogs -lz

main.o: ./archiver/main.c
	$(CC) $(CFLAGS) ./archiver/main.c

paramsAnalysis.o: ./archiver/paramsAnalysis.c
	$(CC) $(CFLAGS) ./archiver/paramsAnalysis.c

compressFile.o: ./archiver/compressFile.c
	$(CC) $(CFLAGS) ./archiver/compressFile.c

hashFunc.o: ./archiver/hashFunc.c
	$(CC) $(CFLAGS) ./archiver/hashFunc.c
