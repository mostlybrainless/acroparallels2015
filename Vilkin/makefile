CC = gcc
CFLAGS = -c
SOURCES = ./log/log.c ./arch/arch.c ./test.c
TOOLS = ./zlib/libz.a
TARGET = archiver

$(TARGET): $(SOURCES:.c = .o)
	$(CC) $^ -o $@ $(TOOLS)

clean:
	rm *.o test
