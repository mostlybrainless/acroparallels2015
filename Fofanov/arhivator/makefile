all: objects/main.o objects/compressor.o objects/my_log.o 
	gcc objects/main.o objects/compressor.o objects/my_log.o -o output -lz

objects/main.o: sources/main.c headers/compressor.h
	gcc -c sources/main.c -o objects/main.o

objects/my_log.o: sources/my_log.c headers/my_log.h
	gcc -c sources/my_log.c -o objects/my_log.o

objects/compressor.o: sources/compressor.c headers/compressor.h
	gcc -c sources/compressor.c -o objects/compressor.o

clear:
	rm output objects/*.o

