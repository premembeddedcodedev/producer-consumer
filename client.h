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
#include "list/list.h"
#include "hospitalexe/clinic.h"
#include "hospitalexe/doctor.h"
#include "hospitalexe/patient.h"

#define WQMAX_ROOM_SIZE 4
#define CBQMAX_ROOM_SIZE 100
#define NUMBER_OF_THREADS 4

#define TRUE 1
#define DEBUG 1 // 0 - no debug message, 1 - main points, 2 - all


#define RECEPTIONTID 1
#define CBQTID 2

#ifndef QueueElement
#define QueueElement void*
#endif

typedef struct low_prio {
	int find_min;
	void *ptr;
}skip_q;

typedef struct doctor_info {
	drevt_aptmt_t doc_deals_with_spec;
	doctor_event_t doc_events;
	ptevt_intr_t interrupter_details;
	int num_patients;
	int doctorid;
	int patientid;
	int interrupt_count;
}doctor_info_t;

typedef struct patient_data {
	int patient_id;
	int idle_time;
	bool interrupted;
	ptevt_register_t patient_reg_info;
	ptevt_dr_t doctor_info;
	
	patient_event_t event_info;
	ptevt_feedback_t doctor_fb;

	struct list_head list;
}patients_info_t;


typedef struct q_lists {
	struct list_head silver_list;
	struct list_head gold_list;
	struct list_head platinum_list;
	struct list_head vip_list;
}q_patients;

typedef struct{
    int seq;
    int capacity;
    int size;
    int pos;
    QueueElement e;
    patients_info_t *pinfo;
    struct list_head list;
} Queue;

typedef struct clinic_info {
	patients_info_t pinfo;
	doctor_info_t dinfo[NUMBER_OF_THREADS + 1];
	Queue *wq;
	Queue *cbq;
	uint8_t thread_id;
	uint32_t seq_id;
	pthread_t reception;
	pthread_t leftroom;
}clinic_info_t;

int process_cbq(clinic_info_t *clinic_info);
int calculate_diag_time(ailment_e diag_type);
patients_info_t *dequeue(Queue *Q, bool is_waitq);
int enqueue(Queue *Q, patients_info_t *pinfo, bool is_waitq);
void *getinstance(void);
#if 0
Queue * initQueue(int max);
QueueElement front(Queue *Q);   
QueueElement tail(Queue* Q);
void dequeue(Queue *Q);
void enqueue(Queue *Q, QueueElement element);
#endif
#endif
