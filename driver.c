#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "include/common.h"
#include "include/list.h"
#include "include/clinic.h"
#include "include/doctor.h"
#include "include/patient.h"

int g_patient_id = 0;

void enqueue_inpt_patient(clinic_info_t *clinic_info, patients_info_t *vacant)
{
	patients_info_t *pinfo = dequeue(clinic_info->cbq, false);

	if(!pinfo)
		return;

	printf("Replace data to CBQ to WQ\n");

	patients_info_t *pinfo_qud = dequeue(clinic_info->wq, true);

	if(!pinfo_qud)
		return;

	if(enqueue(clinic_info->wq, pinfo, true) < 0) {
		enqueue_front(clinic_info->cbq, pinfo, false);
	}

	if(enqueue(clinic_info->wq, pinfo_qud, true) < 0) {
		enqueue_front(clinic_info->cbq, pinfo_qud, false);
	}

	if(enqueue(clinic_info->wq, vacant, true) < 0) {
		enqueue_front(clinic_info->cbq, vacant, false);
	}
}

void wait_for_vip_timesignal(clinic_info_t *clinic_info, patients_info_t *pinfo,
		int tid)
{
	struct timespec start, finish, ts;
	struct timeval tv;
	double elapsed;
	int rc = 0;
	int delaytime = pinfo->idle_time * 1000;

	printf("sleeping: @(%d) time\n", delaytime);
	gettimeofday(&tv, NULL);
	ts.tv_sec = time(NULL) + delaytime / 1000;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (delaytime % 1000);
	ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
	ts.tv_nsec %= (1000 * 1000 * 1000);
	clock_gettime(CLOCK_REALTIME, &start);
	rc = pthread_cond_timedwait(&clinic_info->vip_request,
			&clinic_info->mutex, &ts);
	clock_gettime(CLOCK_REALTIME, &finish);
	if(rc == 0) {
		clinic_info->dinfo[tid].interrupt_count++;
		clinic_info->dinfo[tid].num_interrupts++;
		printf("Signalled properly: doctor: %d interpter count : %d\n",
				tid, clinic_info->dinfo[tid].interrupt_count);
		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
		delaytime = (int) elapsed;
		printf("elapsed : %d\n", delaytime);
		pinfo->idle_time = elapsed;
		pinfo->interrupted_tid = tid;
		pinfo->patient_reg_info.membership = MEMBERSHIP_PLATINUM;
		enqueue_inpt_patient(clinic_info, pinfo);
	} else {
		printf("Timedout from %d thread \n", tid);
		clinic_info->dinfo[tid].num_patients++;
		clinic_info->dinfo[tid].max_patients++;
		free(pinfo);
	}
}

/* Generating specialist for the doctor randomly*/

bool specialist_generation(clinic_info_t *clinic_info, int tid)
{
	if(((clinic_info->dinfo[tid].num_patients) % 4) == 0)
		return true;
	else
		return false;
}

/* fill the doctors and patient's infromation */

void fill_doctor_details(clinic_info_t *clinic_info, int tid, patients_info_t *pinfo)
{
	clinic_info->dinfo[tid].doctorid = tid;
	clinic_info->dinfo[tid].doc_deals_with_spec.specialist_id = 1;
	clinic_info->dinfo[tid].patientid =  pinfo->patient_id;
}

/* fill the interupter infor into doctors and patient's infromation */
void fill_interrupter_details(clinic_info_t *clinic_info, int tid, patients_info_t *pinfo)
{
	clinic_info->dinfo[tid].doc_deals_with_spec.
		patient_id = pinfo->patient_id;
	clinic_info->dinfo[tid].doc_deals_with_spec.ailment
		= pinfo->patient_reg_info.ailment;
	clinic_info->dinfo[tid].doc_deals_with_spec.apt_length
		= pinfo->idle_time + 7;
}

/* Checking how many patient doctors can choose in a day time */
bool is_doctor_max_patients_exceeded(clinic_info_t *clinic_info, int tid)
{
	if((clinic_info->dinfo[tid].max_patients <= DOCTOR1_SERVED_PATIENTS)) 
		return true;

	if((clinic_info->dinfo[tid].max_patients <= DOCTOR2_SERVED_PATIENTS))
		return true;

	if((clinic_info->dinfo[tid].max_patients <= DOCTOR3_SERVED_PATIENTS))
		return true;

	if(clinic_info->dinfo[tid].max_patients <= DOCTOR4_SERVED_PATIENTS)
		return true;

	return false;
}

bool is_intptr_got_scheduled(clinic_info_t *clinic_info, patients_info_t *pinfo,
		int tid)
{
	bool found = false;

	if(pinfo->interrupted_tid == tid) {
		printf("Interrupter patient is schedueled here\n");
		if(clinic_info->dinfo[tid].interrupt_count >= 0)
			clinic_info->dinfo[tid].interrupt_count--;
		found =  true;
	} else if(pinfo->interrupted_tid == 0) {
		found = true;
	} else {
		
		pthread_mutex_lock(&clinic_info->mutex);
		enqueue_inpt_patient(clinic_info, pinfo);
		pthread_mutex_unlock(&clinic_info->mutex);
		
		found = false;
	}

	return found;
}

int get_threadid(clinic_info_t *clinic_info, pthread_t pid)
{
        int i = 0;

        for(i=0; i<NUMBER_OF_THREADS; i++) {
                if(pid == clinic_info->doctorpool[i])
                        return i;
        }

        return 0;
}

/*
 * Submits work to the doctor thread to diagnose the patients in waitroom.
 */

void *doctor_process(void *param)
{
	clinic_info_t *clinic_info = (clinic_info_t *) param;
        pthread_t pid;
        int tid;
	
	while(TRUE) {
		
		pthread_mutex_lock(&clinic_info->mutex);
		/* Dequeuing the waitqueue patients and signaling to CB thread*/
		patients_info_t *pinfo = dequeue(clinic_info->wq, true);
		if(!pinfo) {
			pthread_cond_broadcast(&clinic_info->cbq_request);
			pthread_mutex_unlock(&clinic_info->mutex);
			
			continue;
		}
                pid = pthread_self();
                tid = get_threadid(clinic_info, pid) + 1;

		clinic_info->dinfo[tid].max_patients++;
		if(!is_doctor_max_patients_exceeded(clinic_info, tid)) {
                	printf("doctor : %d interrupted ..!!!\n", tid);
			pthread_mutex_unlock(&clinic_info->mutex);
			
			pthread_exit(0);
			break;
		}

		pthread_mutex_unlock(&clinic_info->mutex);
		

		if(!is_intptr_got_scheduled(clinic_info, pinfo, tid))
			continue;

		printf("Worker thread[%d] : Dq: ailment: %d, membership: %d\n",
				tid, pinfo->patient_reg_info.ailment,
				pinfo->patient_reg_info.membership);

		fill_doctor_details(clinic_info, tid, pinfo);

		if(specialist_generation(clinic_info, tid)) {
			clinic_info->dinfo[tid].num_patients++;
			printf("in specialist consultation...\n");
			clinic_info->dinfo[tid].num_splst_conslnts++;
			fill_interrupter_details(clinic_info, tid, pinfo);
			sleep(pinfo->idle_time + 7);
			free(pinfo);
		} else if(pinfo->patient_reg_info.membership
				== MEMBERSHIP_VIP) {
			clinic_info->dinfo[tid].num_patients++;
			clinic_info->dinfo[tid].num_vip_conslnts++;
			printf("VIP membership executing...\n");
			sleep(pinfo->idle_time);
			free(pinfo);
		} else {
			pthread_mutex_lock(&clinic_info->mutex);
			wait_for_vip_timesignal(clinic_info, pinfo, tid);	
			pthread_mutex_unlock(&clinic_info->mutex);
			
		}
	}
	pthread_exit(0);
}

/* Defining time based on the ailment type
 * */
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


patients_info_t *register_details(void)
{
	patients_info_t *pinfo;

	pinfo = (patients_info_t *) malloc (sizeof(patients_info_t));
	if(!pinfo) {
		printf("Failed in allocations \n");
		return NULL;
	}
	pinfo->patient_id = g_patient_id++;
	printf("\tPatient ID is : %d\t", pinfo->patient_id);
	pinfo->patient_reg_info.ailment = (rand() % AIL_OTHER) + 1;
	printf("\tailment : %d\t", pinfo->patient_reg_info.ailment);
	pinfo->idle_time = calculate_diag_time(pinfo->patient_reg_info.ailment);
	printf("\tDiag time : %d\t", pinfo->idle_time);
	pinfo->patient_reg_info.membership = (rand() % MEMBERSHIP_VIP) + 1;
	printf("\tmembership is : %d\n", pinfo->patient_reg_info.membership);

	return pinfo;
}

bool find_min(Queue *p, patients_info_t *pinfo, int *pos)
{
	Queue  *tmp;
	bool found = false;
	int iteration = 0;

	int find_min = pinfo->patient_reg_info.membership;

	list_for_each_entry(tmp, &p->list, list){
		iteration++;
		if(find_min < tmp->pinfo->patient_reg_info.membership) {
			find_min = tmp->pinfo->patient_reg_info.membership;
			found = true;	
			*pos = iteration;
		}
	}

	return found;
}

void enqueue_cbq(Queue *cbq, Queue *wq)
{
	printf("Triggering backup queue process....\n");

	patients_info_t *pinfo = dequeue(cbq, false);

	if(!pinfo)
		return;

	enqueue(wq, pinfo, true);
}

/*
 * Submits work to the callback queue thread to process the
 * callback room parients
 */
int process_cbq(clinic_info_t *clinic_info)
{
	int rc = 0;

	pthread_mutex_lock(&clinic_info->mutex);
	rc = pthread_cond_wait(&clinic_info->cbq_request, &clinic_info->mutex);
	if(rc == 0) {
		if(clinic_info->cbq->size > 0) {
			/*printf(" from Worker: Signalled properly\n");*/
			enqueue_cbq(clinic_info->cbq, clinic_info->wq); 
		}
	}

	
	pthread_mutex_unlock(&clinic_info->mutex);

	return 0;
}

patients_info_t *dequeue_pos(clinic_info_t *clinic_info, int pos)
{
	Queue *tmp, *person;
	patients_info_t *vacant;
	int check = 0;

	clinic_info->wq->size--;

	list_for_each_entry_safe(person, tmp, &clinic_info->wq->list, list) {
		if(check == pos) {
			/*printf("\tvacant ailment: %d\n",\
			  person->pinfo->patient_reg_info.ailment);*/
			/*printf("\tvacant membership: %d\n", \
			  person->pinfo->patient_reg_info.membership);*/
			vacant = person->pinfo;
			list_del(&person->list);
			free(person);
			return vacant;
		}
		check++;
	}
	return NULL;
}

patients_info_t *find_min_ptr(Queue *p, patients_info_t *pinfo)
{
	patients_info_t *min_ptr = pinfo;
	Queue  *tmp;
	int iteration = 0;

	int find_min = pinfo->patient_reg_info.membership;

	list_for_each_entry(tmp, &p->list, list){
		iteration++;
		if(find_min > tmp->pinfo->patient_reg_info.membership) {
			find_min = tmp->pinfo->patient_reg_info.membership;
			min_ptr = tmp->pinfo; 
		}
	}

	return min_ptr;
}

/*
 * Submits work to the wq thread.
 */
int q_process(clinic_info_t *clinic_info)
{
	patients_info_t *pinfo;
	int pos = 0;

	pinfo = register_details();
	if(!pinfo)
		return -ENOMEM;

	pthread_mutex_lock(&clinic_info->mutex);

	if(clinic_info->wq->size == clinic_info->wq->capacity){
		printf("Wait room is Full. Evicting low prios\n");
		// min should evict requires when you have large buffer
		bool min_find = find_min(clinic_info->wq, pinfo, &pos);
		if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			if(pthread_cond_broadcast(&clinic_info->vip_request) == 0) {
				printf("signal sending from : %s\n", __func__);
				enqueue_front(clinic_info->cbq, pinfo, false);
			} else {
				if(min_find) {
					printf("Found Min element in WQ nodes \n");
					//dq from LL from position
					patients_info_t *temp =
						dequeue_pos(clinic_info, pos);
					//enqueue at last
					enqueue(clinic_info->wq, pinfo, true);
					enqueue(clinic_info->cbq, temp, false);
				} else
					enqueue(clinic_info->cbq, pinfo, false);
			}
		} else {
			enqueue(clinic_info->cbq, pinfo, false);
		}
	} else {
		patients_info_t *min_ptr = find_min_ptr(clinic_info->wq, pinfo);
		enqueue(clinic_info->wq, min_ptr, true);
	}

	
	pthread_mutex_unlock(&clinic_info->mutex);
	printf("pool submitted WQsize: %d CBQSize : %d\n",
			clinic_info->wq->size, clinic_info->cbq->size);
	return 0;
}

/* Registering the patients info randomly
*/
void *register_patients(void *param)
{
	clinic_info_t *clinic_info = (clinic_info_t *)param;

	int val = 0, check = NUMBER_OF_THREADS;
	srand(time(NULL));

	/* Generating ramdom patients and queueing them into WaitQ and CBQ */

	while(clinic_info->max_patients_allowed < MAX_PATIENTS) {
		val = (rand() % 6) + 4;
		printf("\n\n************ Patient is about to enter after "
				"@(%ds) time....\n", val);
		sleep(val);
		q_process(clinic_info);

		while(check >= 1)  {
			printf("Doctor %d: patients served: %d\t, interrupter:"
					"%d\t, specialist_served:%d\t,"
					"vip_served: %d\n", check,
					clinic_info->dinfo[check].num_patients,\
					clinic_info->dinfo[check].num_interrupts, \
					clinic_info->dinfo[check].num_splst_conslnts,\
					clinic_info->dinfo[check].num_vip_conslnts);
			check--;
		}
		check = NUMBER_OF_THREADS;

		clinic_info->max_patients_allowed++;

		printf("****************** Patient left.......\n\n");
	}

	pthread_exit(0);
}

void *process_cbqleftover(void *param)
{
	clinic_info_t *clinic_info = (clinic_info_t *)param;

	while(1)
		process_cbq(clinic_info);

	pthread_exit(0);
}

/*
 * Threads initialising for processing Waitroom Queue
 * and Callback Queue including doctor process.
 */
void threads_init(clinic_info_t *clinic_info)
{
	pthread_mutex_init(&clinic_info->mutex, NULL);
	

	for (int i = 0; i < NUMBER_OF_THREADS; ++i)
		pthread_create(&clinic_info->doctorpool[i+1], NULL, doctor_process,
				(void *) clinic_info);

	if (pthread_cond_init(&clinic_info->vip_request, NULL) != 0) {
		perror("pthread_cond_init() error");
		exit(1);
	}

	if (pthread_cond_init(&clinic_info->cbq_request, NULL) != 0) {
		perror("pthread_cond_init() error");
		exit(1);
	}

	printf("created threads successfully\n");
}

/*
 * freeing thread and its object resources.
 */
void threads_clean(clinic_info_t *clinic_info)
{
	if (pthread_cond_destroy(&clinic_info->vip_request) != 0) {                                       
		perror("pthread_cond_destroy() vip error");                                     
		exit(2);                                                                    
	}       

	if (pthread_cond_destroy(&clinic_info->cbq_request) != 0) {
		perror("pthread_cond_destroy() cbq error");
		exit(2);
	}
	for (int i = 0; i < NUMBER_OF_THREADS; ++i) 
		pthread_join(clinic_info->doctorpool[i+1],NULL);

	free(clinic_info);

	printf("End of execution :)\n");
}

/* Initialising all the clinic data structure for patient and doctor info
 * */

clinic_info_t *clinic_init(void)
{
	clinic_info_t *clinic_info;

	DEBUGLOG_INIT("afile.log");
        DEBUGLOG_LOG(1, "the value is: %d", g_patient_id);

	/* Intialising the clinic data structure where it has info of 
	 * patients and doctor data structures*/

	clinic_info = (clinic_info_t *) malloc (sizeof(clinic_info_t));
	if(!clinic_info) {
		printf("No memory allocated \n");
		return NULL;
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
			process_cbqleftover, (void *) clinic_info);

	sleep(1);

	pthread_join(clinic_info->reception, NULL);
	pthread_join(clinic_info->leftroom, NULL);

	/* freeing up resources here */

	threads_clean(clinic_info);

	return clinic_info;
}

