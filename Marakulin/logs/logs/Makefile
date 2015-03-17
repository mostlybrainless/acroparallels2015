libLogs.so: logs.o
	g++ -shared -o libLogs.so logs.o
logs.o: logs.cpp
	g++ -c -fPIC logs.cpp
clean:
	rm -f *.o
