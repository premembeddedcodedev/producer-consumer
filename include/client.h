#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "common.h"


/* User has to change below values to set the waitQ and CBQ size
 * and doctor max attend patients */
#define WQMAX_ROOM_SIZE 4
#define CBQMAX_ROOM_SIZE 100
#define NUMBER_OF_THREADS 4
#define MAX_PATIENTS 100 * NUMBER_OF_THREADS

#define DQ_SIZE MAX_PATIENTS
#define PQ_SIZE MAX_PATIENTS

/* Doctor can manage per day patients using below macros*/
#define DOCTOR1_SERVED_PATIENTS 1000
#define DOCTOR2_SERVED_PATIENTS 1000
#define DOCTOR3_SERVED_PATIENTS 1000
#define DOCTOR4_SERVED_PATIENTS 1000

#endif
