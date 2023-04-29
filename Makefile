# makefile for thread pool
#
CC=gcc
CFLAGS=-Wall -g
PTHREADS=-lpthread -lrt

all: client.o driver.o queue.o debuglog.o
	$(CC) $(CFLAGS) -o test client.o driver.o queue.o debuglog.o $(PTHREADS)

client.o: client.c include/debuglog.h include/doctor.h include/patient.h include/clinic.h include/client.h include/ include/common.h
	$(CC) $(CFLAGS) -c client.c $(PTHREADS)

driver.o: driver.c include/debuglog.h include/doctor.h include/patient.h include/clinic.h include/client.h include/ include/common.h
	$(CC) $(CFLAGS) -c driver.c $(PTHREADS)

queue.o: queue.c include/debuglog.h include/client.h include/ include/doctor.h include/patient.h include/clinic.h include/common.h
	$(CC) $(CFLAGS) -c queue.c $(PTHREADS)

debuglog.o: include/debuglog.h debuglog.c
	$(CC) $(CFLAGS) -c debuglog.c $(PTHREADS)
clean:
	rm -rf *.o
	rm -rf test
