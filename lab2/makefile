all: http-server

http-server: libhttp.o main.o
	gcc -o http-server libhttp.o main.o

libhttp.o: libhttp.c
	gcc -c libhttp.c

main.o: main.c
	gcc -c main.c

clean:
	rm -rf http-server libhttp.o main.o