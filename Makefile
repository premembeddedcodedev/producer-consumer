# makefile for thread pool
#
CC=gcc
CFLAGS=-Wall -g
PTHREADS=-lpthread -lrt

all: client.o driver.o queue.o
	$(CC) $(CFLAGS) -o test client.o driver.o queue.o $(PTHREADS)

client.o: client.c hospitalexe/doctor.h hospitalexe/patient.h hospitalexe/clinic.h client.h queue.h common.h
	$(CC) $(CFLAGS) -c client.c $(PTHREADS)

driver.o: driver.c driver.h hospitalexe/doctor.h hospitalexe/patient.h hospitalexe/clinic.h client.h queue.h common.h
	$(CC) $(CFLAGS) -c driver.c $(PTHREADS)

queue.o: queue.c client.h queue.h hospitalexe/doctor.h hospitalexe/patient.h hospitalexe/clinic.h common.h
	$(CC) $(CFLAGS) -c queue.c $(PTHREADS)
clean:
	rm -rf *.o
	rm -rf test
