#include <semaphore.h>
// function prototypes
#include <stdint.h>
#include <stdbool.h>
#include "list/list.h"
#include "hospitalexe/clinic.h"
#include "hospitalexe/doctor.h"
#include "hospitalexe/patient.h"
#include "client.h"

int q_process(clinic_info_t *clinic_info);
void *worker(void *param);
void threads_init(clinic_info_t *clinic_info);
void threads_clean(clinic_info_t *clinic_info);
