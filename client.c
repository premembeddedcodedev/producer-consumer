#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "include/queue.h"
#include "include/driver.h"
#include "include/client.h"

clinic_info_t *clinic_info;

int calculate_diag_time(ailment_e diag_type)
{
	int value = 0;

	switch(diag_type) {
		case AIL_CHECKUP:
			value = 12;
			break;
		case AIL_GENERAL_MALAISE:
			value = 10;
			break;
		case AIL_FEVER:
			value = 8;
			break;
		case AIL_PHYSICAL_INJURY:
			value = 6;
			break;
		case AIL_OTHER:
			value = 4;
			break;
		default:
			break;
	}

	return value;
}

void *register_patients(void *param)
{
	int val = 0, check = NUMBER_OF_THREADS;
	srand(time(NULL));

	/* Generating ramdom patients and queueing them into WaitQ and CBQ */

	while(1) {
		val = (rand() % 6) + 4;
		printf("\n\n************ Patient is about to enter after \
				@(%ds) time....\n", val);
		sleep(val);
		q_process(clinic_info);

		while(check >= 1)  {
			printf("Doctor %d: patients served: %d, interrupter:\
					%d\n", check,\
				clinic_info->dinfo[check].num_patients,\
				clinic_info->dinfo[check].interrupt_count);
			check--;
		}
		check = NUMBER_OF_THREADS;

		printf("****************** Patient left.......\n\n");
	}

	pthread_exit(0);
}

void *process_leftover(void *param)
{
	while(1) 
		process_cbq(clinic_info);

	pthread_exit(0);
}

void *getinstance(void)
{
	return clinic_info;
}

int main()
{
	/* Intialising the clinic data structure where it has info of 
	 * patients and doctor data structures*/

	clinic_info = (clinic_info_t *) malloc (sizeof(clinic_info_t));
	if(!clinic_info) {
		printf("No memory allocated \n");
		return -ENOMEM;
	}

	memset(clinic_info, 0, sizeof(clinic_info_t));

	/* preparing WaitQueue size and callback size defined in header file */

	clinic_info->wq = initQueue(WQMAX_ROOM_SIZE);
	clinic_info->cbq  = initQueue(CBQMAX_ROOM_SIZE);

	printf("main %p : %p\n", clinic_info->wq, clinic_info->cbq);
	threads_init(clinic_info);

	/* Creating 2 threads for enqueing the waitroom patients and
	 * callback room patients
	 * */

	pthread_create(&clinic_info->reception, NULL,
			register_patients, (void *) clinic_info);
	pthread_create(&clinic_info->leftroom, NULL,
			process_leftover, (void *) clinic_info);

	sleep(1);

	/* freeing up resources here */
	
	pthread_join(clinic_info->reception, NULL);
	pthread_join(clinic_info->leftroom, NULL);

	threads_clean(clinic_info);

	return 0;
}
