#ifndef __COMMON_H__
#define __COMMON_H__
#include "list/list.h"
#include "hospitalexe/clinic.h"
#include "hospitalexe/doctor.h"
#include "hospitalexe/patient.h"

#define WQMAX_ROOM_SIZE 4
#define CBQMAX_ROOM_SIZE 100
#define NUMBER_OF_THREADS 4

#define DOCTOR1_SERVED_PATIENTS 2
#define DOCTOR2_SERVED_PATIENTS 2
#define DOCTOR3_SERVED_PATIENTS 2
#define DOCTOR4_SERVED_PATIENTS 2

typedef struct doctor_info {
	drevt_aptmt_t doc_deals_with_spec;
	doctor_event_t doc_events;
	ptevt_intr_t interrupter_details;
	int num_patients;
	int doctorid;
	int patientid;
	int interrupt_count;
	int max_patients;
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

#if 0
#include "queue.h"

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
#endif

#endif
