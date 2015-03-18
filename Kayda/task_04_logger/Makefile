
CFLAGS= -pthread -O0


logger: main.o logger.o
	gcc $(CFLAGS) $^ -o $@

main.o: main.c
	gcc $(CFLAGS) -c $< -o $@

logger.o: logger.c logger.h
	gcc $(CFLAGS) -c $< -o $@


clean: 
	rm -rf *.o logger

