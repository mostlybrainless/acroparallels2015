CC = gcc
CFLAGS = -c -Wall
SOURCES = log.c test.c
TARGET = test

$(TARGET): $(SOURCES:.c = .o)
	$(CC) $^ -o $@

clean:
	rm *.o test
