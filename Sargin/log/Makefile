CC=gcc
CFLAGS=-I.
DEPS = logger.h
OBJ = main.o logger.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

loggermake: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o

