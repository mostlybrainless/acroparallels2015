CC = gcc
INCLUDE = ./include
SOURCES = ./archivator/archivator.c ./log/logger.c ./main.c

all: archiver

archiver: $(SOURCES)
	$(CC) $^ -o $@ -I$(INCLUDE) -lz

clean:
	rm -f *.o

