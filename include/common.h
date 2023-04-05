#ifndef __COMMON_H__
#define __COMMON_H__
#include "list.h"
#include "clinic.h"
#include "doctor.h"
#include "patient.h"

/* User has to change below values to set the waitQ and CBQ size
 * and doctor max attend patients */

#define WQMAX_ROOM_SIZE 4
#define CBQMAX_ROOM_SIZE 100
#define NUMBER_OF_THREADS 4

#define DOCTOR1_SERVED_PATIENTS 1000
#define DOCTOR2_SERVED_PATIENTS 1000
#define DOCTOR3_SERVED_PATIENTS 1000
#define DOCTOR4_SERVED_PATIENTS 1000

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
	struct list_head list;
}patients_info_t;

#endif
