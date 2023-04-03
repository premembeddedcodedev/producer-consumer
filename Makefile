# makefile for thread pool
#
CC=gcc
CFLAGS=-Wall -g
PTHREADS=-lpthread -lrt

all: client.o threadpool.o
	$(CC) $(CFLAGS) -o test client.o threadpool.o $(PTHREADS)

client.o: client.c hospitalexe/doctor.h hospitalexe/patient.h hospitalexe/clinic.h client.h
	$(CC) $(CFLAGS) -c client.c $(PTHREADS)

threadpool.o: threadpool.c threadpool.h hospitalexe/doctor.h hospitalexe/patient.h hospitalexe/clinic.h client.h 
	$(CC) $(CFLAGS) -c threadpool.c $(PTHREADS)

clean:
	rm -rf *.o
	rm -rf test
