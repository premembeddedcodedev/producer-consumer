#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "clinic.h"
#include "doctor.h"
#include "patient.h"
#include "client.h"

// function prototypes
int q_process(clinic_info_t *clinic_info);
void *worker(void *param);
void threads_init(clinic_info_t *clinic_info);
void threads_clean(clinic_info_t *clinic_info);
