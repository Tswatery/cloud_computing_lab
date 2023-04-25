all: http-server

http-server: libhttp.o main.o server.o
	gcc -o http-server libhttp.o server.o main.o

libhttp.o: libhttp.c
	gcc -c libhttp.c

main.o: main.c
	gcc -c main.c

server.o: server.c
	gcc -c server.c

clean:
	rm -rf http-server libhttp.o server.o main.o