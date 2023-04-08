#ifndef __COMMON_H__
#define __COMMON_H__
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <semaphore.h>
#include "list.h"
#include "client.h"
#include "clinic.h"
#include "doctor.h"
#include "patient.h"

#define TRUE 1

#ifndef QueueElement
#define QueueElement void*
#endif

typedef struct patient_data {
	int patient_id;
	int idle_time;
	int interrupted_tid;
	ptevt_register_t patient_reg_info;
	struct list_head list;
}patients_info_t;

typedef struct doctor_info {
	drevt_aptmt_t doc_deals_with_spec;
	doctor_event_t doc_events;
	ptevt_intr_t interrupter_details;
	int num_patients;
	int doctorid;
	int patientid;
	int interrupt_count;
	int num_interrupts;
	int max_patients;
	int num_vip_conslnts;
	int num_splst_conslnts;
}doctor_info_t;

typedef struct{
    int seq;
    int capacity;
    int size;
    int pos;
    QueueElement e;
    patients_info_t *pinfo;
    doctor_info_t *dinfo;
    struct list_head list;
} Queue;


/* Clinic data structure */
typedef struct clinic_info {
	Queue *wq;
	Queue *cbq;
	Queue *dq;
	Queue *pq;
	pthread_t reception;
	pthread_t leftroom;
	pthread_mutex_t mutex;
	pthread_cond_t vip_request;
	pthread_cond_t cbq_request;
	sem_t semaphore;
	int max_patients_allowed;
	int thread_num[NUMBER_OF_THREADS + 1];
	uint8_t doctor_max_patients;
	patients_info_t pinfo;
	pthread_t doctorpool[NUMBER_OF_THREADS + 1];
	doctor_info_t dinfo[NUMBER_OF_THREADS + 1];
}clinic_info_t;

clinic_info_t *clinic_init(void);

int enqueue(Queue *Q, patients_info_t *pinfo, bool is_waitq);
int enqueue_dinfo(Queue *Q, doctor_info_t *pinfo, bool is_waitq);

patients_info_t *dequeue(Queue *Q, bool is_waitq);

Queue *dequeue_ptr(Queue *Q);

int enqueue_front(Queue *Q, patients_info_t *pinfo, bool is_waitq);

Queue * initQueue(int max);

#endif
