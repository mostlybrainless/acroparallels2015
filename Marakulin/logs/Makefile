LIBPATH=../archiver/libs/libLogs.so
libLogs.so: logs.o
	gcc -shared -o libLogs.so logs.o
	@if [ -f $(LIBPATH) ]; then rm $(LIBPATH); fi 
	ln libLogs.so $(LIBPATH)
logs.o: logs.c
	gcc -c -fPIC logs.c
clean:
	rm -f *.o
