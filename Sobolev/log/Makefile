libLog.so: log.o
	gcc -shared -o libLog.so log.o
log.o: log.c
	gcc -c -fPIC log.c
clean:
	rm -f *.o
