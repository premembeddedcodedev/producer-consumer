/**
 * Implementation of thread pool.
 */
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "driver.h"
#include "queue.h"
#include "client.h"
#include <pthread.h>
#include "list/list.h"
#include "hospitalexe/clinic.h"
#include "hospitalexe/doctor.h"
#include "hospitalexe/patient.h"

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
	} else
		clinic_info->dinfo[tid].num_patients++;
}

bool specialist_generation(clinic_info_t *clinic_info, int tid)
{
	if(((clinic_info->dinfo[tid].num_patients) % 4) == 0)
		return true;
	else
		return false;
}

void fill_doctor_details(clinic_info_t *clinic_info, int tid)
{
	clinic_info->dinfo[tid].doctorid = tid;
	clinic_info->dinfo[tid].doc_deals_with_spec.specialist_id = 1;
}

bool is_doctor_max_patients_exceeded(clinic_info_t *clinic_info, int tid)
{
	//return true;

	//for(int i=1; i<NUMBER_OF_THREADS+1; i++) {

	if((tid <= (NUMBER_OF_THREADS + 1)) && (clinic_info->dinfo[tid].max_patients < DOCTOR1_SERVED_PATIENTS)) 
			return true;

	if((tid <= (NUMBER_OF_THREADS + 1)) && (clinic_info->dinfo[tid].max_patients < DOCTOR2_SERVED_PATIENTS))
			return true;

	if((tid <= (NUMBER_OF_THREADS + 1)) && (clinic_info->dinfo[tid].max_patients < DOCTOR3_SERVED_PATIENTS))
			return true;

	if((tid <= (NUMBER_OF_THREADS + 1)) && (clinic_info->dinfo[tid].max_patients < DOCTOR4_SERVED_PATIENTS))
			return true;
	//}

	return false;
}

void *doctor_process(void *param)
{
	int tid = *((int *) param);
	clinic_info_t *clinic_info = (clinic_info_t *)getinstance();

	//while(is_doctor_max_patients_exceeded(clinic_info, tid)) {
	while(TRUE) {
		sem_wait(&clinic_info->semaphore);
		pthread_mutex_lock(&clinic_info->mutex);
		clinic_info_t *clinic_info = (clinic_info_t *)getinstance();
		patients_info_t *pinfo = dequeue(clinic_info->wq, true);
		if(!pinfo) {
			pthread_cond_broadcast(&clinic_info->cbq_request);
			pthread_mutex_unlock(&clinic_info->mutex);
			sem_post(&clinic_info->semaphore);
			continue;
		}
		pthread_mutex_unlock(&clinic_info->mutex);
		sem_post(&clinic_info->semaphore);

		printf("Worker thread[%d] : Dq: ailment: %d, membership: %d\n",
				tid, pinfo->patient_reg_info.ailment,
				pinfo->patient_reg_info.membership);

		fill_doctor_details(clinic_info, tid);
		clinic_info->dinfo[tid].patientid =  pinfo->patient_id;

		if(specialist_generation(clinic_info, tid)) {
			clinic_info->dinfo[tid].num_patients++;
			printf("in specialist consultation...\n");
			sleep(pinfo->idle_time + 7);
			clinic_info->dinfo[tid].doc_deals_with_spec.\
				patient_id = pinfo->patient_id;
			clinic_info->dinfo[tid].doc_deals_with_spec.ailment\
				= pinfo->patient_reg_info.ailment;
			clinic_info->dinfo[tid].doc_deals_with_spec.apt_length\
				= pinfo->idle_time + 7;
		} else if(pinfo->patient_reg_info.membership
				== MEMBERSHIP_VIP) {
			clinic_info->dinfo[tid].num_patients++;
			printf("VIP membership executing...\n");
			sleep(pinfo->idle_time);
		} else {
			pthread_mutex_lock(&clinic_info->mutex);
			wait_for_vip_timesignal(clinic_info, pinfo, tid);	
			pthread_mutex_unlock(&clinic_info->mutex);
			sem_post(&clinic_info->semaphore);
		}
		clinic_info->dinfo[tid].max_patients++;
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

int process_cbq(clinic_info_t *clinic_info)
{
	int rc = 0;

	pthread_mutex_lock(&clinic_info->mutex);
	rc = pthread_cond_wait(&clinic_info->cbq_request, &clinic_info->mutex);

	if(rc == 0) {
		if(clinic_info->cbq->size > 0) {
			//printf(" from Worker: Signalled properly\n");
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

	//printf("*** Psize is now: %d\n", p->size);

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
#if 0
void swap(int *to, int* from)
{
	int temp;

	temp = *from;
	*from = *to;
	*to = temp;
}

void sort_queue(struct list_head *program_list)
{
	Queue *p, *q;
	struct list_head *new;

	list_for_each_entry(p, program_list, list){
		new = program_list->next;	
		list_for_each_entry(q, new, list){
			if(p->pinfo->patient_reg_info.ailment < q->pinfo->patient_reg_info.ailment) {
				//swap(&p->pinfo->patient_reg_info.ailment, &q->pinfo->patient_reg_info.ailment);
				printf("\tailment: %d\t", p->pinfo->patient_reg_info.ailment);
				printf("\tmembership: %d\n",\
						q->pinfo->patient_reg_info.membership);
				printf("\tailment: %d\t", p->pinfo->patient_reg_info.ailment);
				printf("\tmembership: %d\n",\
						q->pinfo->patient_reg_info.membership);
			}
		}
	}

}
#endif
patients_info_t *find_min_ptr(Queue *p, patients_info_t *pinfo)
{
	patients_info_t *min_ptr = pinfo;
	Queue  *tmp;
	int iteration = 0;

	int find_min = pinfo->patient_reg_info.membership;
	//printf("============> finding min default value: %d\n", find_min);

	list_for_each_entry(tmp, &p->list, list){
		iteration++;
		if(find_min > tmp->pinfo->patient_reg_info.membership) {
			find_min = tmp->pinfo->patient_reg_info.membership;
			min_ptr = tmp->pinfo; 
		}
	}
	//printf("============> finding min default value: %d\n", min_ptr->patient_reg_info.membership);

	return min_ptr;
}

/*
 * Submits work to the pool.
 */
int q_process(clinic_info_t *clinic_info)
{
	patients_info_t *pinfo;
	int pos = 0;

	//if vip thread is executing by the time new patient arrives

	pinfo = register_details();

	pthread_mutex_lock(&clinic_info->mutex);

	if(clinic_info->wq->size == clinic_info->wq->capacity){
		printf("Wait room is Full. Evicting low prios\n");
		// this requires when you have large buffer
		bool min_find = find_min(clinic_info->wq, pinfo, &pos);
		if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			if(pthread_cond_broadcast(&clinic_info->vip_request) == 0) {
				printf("signal successful from : %s\n", __func__);
				enqueue_front(clinic_info->cbq, pinfo, false);
			} else {
				if(min_find) {
					printf("Found Min element in WQ nodes \n");
					//dq from LL from position
					patients_info_t *temp = dequeue_pos(clinic_info, pos);
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
		//patients_info_t *min_ptr = find_min_ptr(clinic_info->wq, pinfo);
		//enqueue(clinic_info->wq, min_ptr, true);
		enqueue(clinic_info->wq, pinfo, true);
	}

	sem_post(&clinic_info->semaphore);
	pthread_mutex_unlock(&clinic_info->mutex);
	printf("pool submitted WQsize: %d CBQSize : %d\n", clinic_info->wq->size, clinic_info->cbq->size);
	return 0;
}

void threads_init(clinic_info_t *clinic_info)
{
	clinic_info->thread_num = (int *) malloc(sizeof(int));
	if(!clinic_info->thread_num)
		return;

	printf("Address of thread_num: %p\n", clinic_info->thread_num);

	*(clinic_info->thread_num) = 1;

	pthread_mutex_init(&clinic_info->mutex, NULL);
	sem_init(&clinic_info->semaphore, 0, NUMBER_OF_THREADS);

	for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
		*(clinic_info->thread_num) = i+1;
		pthread_create(&clinic_info->doctorpool, NULL, doctor_process,
				(void *) clinic_info->thread_num);
		printf("Created %d thread successfully\n", *clinic_info->thread_num);
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
	pthread_join(clinic_info->doctorpool,NULL);
	if (DEBUG) printf("End of execution :)\n");
}
