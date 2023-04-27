all: http-server

http-server: libhttp.o main.o libget.o libpost.o
	gcc -o http-server libhttp.o libget.o libpost.o main.o

libhttp.o: libhttp.c
	gcc -c libhttp.c

main.o: main.c
	gcc -c main.c

libget.o: libget.c
	gcc -c libget.c

libpost.o: libpost.c
	gcc -c libpost.c

clean:
	rm -rf http-server libhttp.o libget.o main.o