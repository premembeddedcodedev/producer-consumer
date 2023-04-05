#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "queue.h"
#include "list.h"
#include "clinic.h"
#include "doctor.h"
#include "patient.h"
#include <semaphore.h>

#define TRUE 1

/* Clinic data structure */

typedef struct clinic_info {
	Queue *wq;
	Queue *cbq;
	pthread_t reception;
	pthread_t leftroom;
	pthread_mutex_t mutex;
	pthread_cond_t vip_request;
	pthread_cond_t cbq_request;
	sem_t semaphore;
	int thread_num[NUMBER_OF_THREADS + 1];
	uint8_t doctor_max_patients;
	patients_info_t pinfo;
	pthread_t doctorpool[NUMBER_OF_THREADS + 1];
	doctor_info_t dinfo[NUMBER_OF_THREADS + 1];
}clinic_info_t;

int process_cbq(clinic_info_t *clinic_info);
int calculate_diag_time(ailment_e diag_type);
void *getinstance(void);
#endif
