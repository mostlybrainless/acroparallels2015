CC=g++
CFLAGS=-c -std=c++11

all: main

main: main.o node.o compress.o fileController.o decompress.o
	$(CC) main.o node.o compress.o decompress.o fileController.o -o main

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

node.o: node.cpp
	$(CC) $(CFLAGS) node.cpp

compress.o: compress.cpp
	$(CC) $(CFLAGS) compress.cpp

decompress.o: decompress.cpp
	$(CC) $(CFLAGS) decompress.cpp

fileController.o: fileController.cpp
	$(CC) $(CFLAGS) fileController.cpp

clean:
	rm -rf *.o main