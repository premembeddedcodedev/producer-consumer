#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "include/queue.h"
#include "include/client.h"
#include <pthread.h>
#include "include/list.h"
#include "include/clinic.h"
#include "include/doctor.h"
#include "include/patient.h"

int patient_id = 0;

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
		printf("Signalled properly: doctor: %d interpter count : %d\n",
				tid, clinic_info->dinfo[tid].interrupt_count);
		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
		delaytime = (int) elapsed;
		printf("elapsed : %d\n", delaytime);
		pinfo->idle_time = elapsed;
		pinfo->interrupted = true;
		pinfo->patient_reg_info.membership = MEMBERSHIP_PLATINUM;
		enqueue_inpt_patient(clinic_info, pinfo);
	} else {
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

/*
 * Submits work to the doctor thread to diagnose the patients in waitroom.
 */

void *doctor_process(void *param)
{
	int tid = *((int *) param);

	while(TRUE) {
		clinic_info_t *clinic_info = (clinic_info_t *)getinstance();
		sem_wait(&clinic_info->semaphore);
		pthread_mutex_lock(&clinic_info->mutex);
		/* Dequeuing the waitqueue patients and signaling to CB thread*/
		patients_info_t *pinfo = dequeue(clinic_info->wq, true);
		if(!pinfo) {
			pthread_cond_broadcast(&clinic_info->cbq_request);
			pthread_mutex_unlock(&clinic_info->mutex);
			sem_post(&clinic_info->semaphore);
			continue;
		}

		clinic_info->dinfo[tid].max_patients++;
		if(!is_doctor_max_patients_exceeded(clinic_info, tid)) {
			pthread_mutex_unlock(&clinic_info->mutex);
			sem_post(&clinic_info->semaphore);
			pthread_exit(0);
			break;
		}
		pthread_mutex_unlock(&clinic_info->mutex);
		sem_post(&clinic_info->semaphore);

		printf("Worker thread[%d] : Dq: ailment: %d, membership: %d\n",
				tid, pinfo->patient_reg_info.ailment,
				pinfo->patient_reg_info.membership);

		fill_doctor_details(clinic_info, tid, pinfo);

		if(specialist_generation(clinic_info, tid)) {
			clinic_info->dinfo[tid].num_patients++;
			printf("in specialist consultation...\n");
			fill_interrupter_details(clinic_info, tid, pinfo);
			sleep(pinfo->idle_time + 7);
			free(pinfo);
		} else if(pinfo->patient_reg_info.membership
				== MEMBERSHIP_VIP) {
			clinic_info->dinfo[tid].num_patients++;
			printf("VIP membership executing...\n");
			sleep(pinfo->idle_time);
			free(pinfo);
		} else {
			pthread_mutex_lock(&clinic_info->mutex);
			wait_for_vip_timesignal(clinic_info, pinfo, tid);	
			pthread_mutex_unlock(&clinic_info->mutex);
			sem_post(&clinic_info->semaphore);
		}
	}
	pthread_exit(0);
}

patients_info_t *register_details(void)
{
	patients_info_t *pinfo;

	pinfo = (patients_info_t *) malloc (sizeof(patients_info_t));
	if(!pinfo) {
		printf("Failed in allocations \n");
		return NULL;
	}
	pinfo->patient_id = patient_id++;
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

	sem_post(&clinic_info->semaphore);
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
				printf("signal successful from : %s\n", __func__);
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

	sem_post(&clinic_info->semaphore);
	pthread_mutex_unlock(&clinic_info->mutex);
	printf("pool submitted WQsize: %d CBQSize : %d\n",
			clinic_info->wq->size, clinic_info->cbq->size);
	return 0;
}

/*
 * Threads initialising for processing Waitroom Queue
 * and Callback Queue including doctor process.
 */

void threads_init(clinic_info_t *clinic_info)
{
	int j = 1;

	printf("Address of thread_num: %p\n", clinic_info->thread_num);

	clinic_info->thread_num[0] = 1;

	pthread_mutex_init(&clinic_info->mutex, NULL);
	sem_init(&clinic_info->semaphore, 0, NUMBER_OF_THREADS);

	for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
		clinic_info->thread_num[i] = j;
		pthread_create(&clinic_info->doctorpool[j], NULL, doctor_process,
				(void *) &clinic_info->thread_num[i]);
		printf("Created %d thread successfully\n", clinic_info->thread_num[i]);
		j++;
	}

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
