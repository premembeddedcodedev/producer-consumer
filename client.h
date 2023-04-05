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
#include "list/list.h"
#include "hospitalexe/clinic.h"
#include "hospitalexe/doctor.h"
#include "hospitalexe/patient.h"
#include <semaphore.h>


#define TRUE 1
#define DEBUG 1 // 0 - no debug message, 1 - main points, 2 - all

#define RECEPTIONTID 1
#define CBQTID 2

typedef struct low_prio {
	int find_min;
	void *ptr;
}skip_q;

typedef struct clinic_info {
	int thread_num[NUMBER_OF_THREADS + 1];
	patients_info_t pinfo;
	doctor_info_t dinfo[NUMBER_OF_THREADS + 1];
	Queue *wq;
	Queue *cbq;
	uint8_t thread_id;
	uint32_t seq_id;
	uint8_t doctor_max_patients;
	pthread_t reception;
	pthread_t leftroom;
	pthread_t doctorpool[NUMBER_OF_THREADS + 1];
	pthread_mutex_t mutex;
	pthread_cond_t vip_request;
	pthread_cond_t cbq_request;
	sem_t semaphore;
}clinic_info_t;

int process_cbq(clinic_info_t *clinic_info);
int calculate_diag_time(ailment_e diag_type);
void *getinstance(void);
#endif
