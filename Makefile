# makefile for thread pool
#
CC=gcc
CFLAGS=-Wall -g
PTHREADS=-lpthread -lrt

all: debuglog.o main.o
	$(CC) $(CFLAGS) -o test debuglog.o main.o $(PTHREADS)


debuglog.o: debuglog.h main.c
	$(CC) $(CFLAGS) -c main.c $(PTHREADS)

debuglog.o: debuglog.h debuglog.c
	$(CC) $(CFLAGS) -c debuglog.c $(PTHREADS)

clean:
	rm -rf *.o
	rm -rf test
