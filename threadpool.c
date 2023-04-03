/**
 * Implementation of thread pool.
 */

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"
#include "client.h"
#include <pthread.h>
struct task worktodo;
struct threadpool *pool;
time_t begin;

pthread_t doctors; // the worker bee
int patient_id = 0;
pthread_cond_t vip_request  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cbq_request  = PTHREAD_COND_INITIALIZER;

void enqueue_front(Queue *Q, patients_info_t *pinfo, bool is_waitq)
{
        Queue* newQ;
        if(Q->size == Q->capacity){
                printf(" %s : Queue is Full. No element added.\n", (is_waitq ? "WQ": "CBQ"));
                return -1;
        }
        else{
                Q->size++;
                printf("number of elements in %s are : %d\n", (is_waitq ? "WQ": "CBQ"), Q->size);
                newQ = (Queue*) malloc(sizeof(Queue));
                newQ->pinfo = pinfo;
                /* add to the list front */
                list_add(&(newQ->list), &(Q->list));
                return 0;
        }
}

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

void wait_for_vip_timesignal(clinic_info_t *clinic_info, patients_info_t *pinfo, int tid)
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
	rc = pthread_cond_timedwait(&vip_request, &pool->mutex, &ts);
	clock_gettime(CLOCK_REALTIME, &finish);
	if(rc == 0) {
		clinic_info->dinfo[tid].interrupt_count++;
		printf("Signalled properly: doctor: %d interpter count : %d\n", tid, clinic_info->dinfo[tid].interrupt_count);
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
	//clinic_info->dinfo[tid].doctorid = tid;
        //clinic_info->dinfo[tid].doc_deals_with_spec.specialist_id = 1;
}

void *worker(void *param)
{
	int tid =  (int) param;
	int  i = 0;

	while (TRUE) {
		sem_wait(&pool->semaphore);
		pthread_mutex_lock(&pool->mutex);
		clinic_info_t *clinic_info = (clinic_info_t *)getinstance();
		patients_info_t *pinfo = dequeue(clinic_info->wq, true);
		if(!pinfo) {
			pthread_cond_broadcast(&cbq_request);
			pthread_mutex_unlock(&pool->mutex);
			sem_post(&pool->semaphore);
			continue;
		}
		pthread_mutex_unlock(&pool->mutex);
		sem_post(&pool->semaphore);
		
		printf("Worker thread[%d] : Dq: ailment: %d, membership: %d\n", tid, pinfo->patient_reg_info.ailment, pinfo->patient_reg_info.membership);

		fill_doctor_details(clinic_info, tid);
		clinic_info->dinfo[tid].patientid =  pinfo->patient_id;
		
		if(specialist_generation(clinic_info, tid)) {
			clinic_info->dinfo[tid].num_patients++;
			printf("in specialist consultation...\n");
			sleep(pinfo->idle_time + 7);
			//clinic_info->dinfo[tid].doc_deals_with_spec.patient_id = pinfo->patient_id;
        		//clinic_info->dinfo[tid].doc_deals_with_spec.ailment = pinfo->patient_reg_info.ailment;
        		//clinic_info->dinfo[tid].doc_deals_with_spec.apt_length = pinfo->idle_time + 7;
		} else if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			clinic_info->dinfo[tid].num_patients++;
			printf("VIP membership executing...\n");
			sleep(pinfo->idle_time);
		} else {
			pthread_mutex_lock(&pool->mutex);
			wait_for_vip_timesignal(clinic_info, pinfo, tid);	
			pthread_mutex_unlock(&pool->mutex);
			sem_post(&pool->semaphore);
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
	skip_q intpt_min;
	bool found = false;
	int iteration = 0;

	memset(&intpt_min, 0, sizeof(skip_q)); //Redundant

	int find_min = pinfo->patient_reg_info.membership;
	intpt_min.ptr = pinfo;

	list_for_each_entry(tmp, &p->list, list){
		iteration++;
		if(find_min < tmp->pinfo->patient_reg_info.membership) {
			find_min = tmp->pinfo->patient_reg_info.membership;
			intpt_min.ptr = tmp->pinfo;
			found = true;	
			*pos = iteration;
		}
	}
	
	//if(find_min == MEMBERSHIP_VIP)
	//	found = true;

	return found;
}

void enqueue_cbq(Queue *cbq, Queue *wq)
{
	Queue *person, *tmp;

	printf("Triggering backup queue process....\n");

	patients_info_t *pinfo = dequeue(cbq, false);

	if(!pinfo)
		return;

	enqueue(wq, pinfo, true);
}

int process_cbq(clinic_info_t *clinic_info)
{
	struct timeval  tv;
	struct timespec ts;
	int rc = 0;

	pthread_mutex_lock(&pool->mutex);
#if 0
	gettimeofday(&tv, NULL);
	ts.tv_sec = time(NULL) + 1000 / 1000;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (1000 % 1000);
	ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
	ts.tv_nsec %= (1000 * 1000 * 1000);
#endif
	//rc = pthread_cond_timedwait(&cbq_request, &pool->mutex, &ts);
	rc = pthread_cond_wait(&cbq_request, &pool->mutex);
	switch(rc) {
		case 0:
			if(clinic_info->cbq->size > 0) {
				//printf(" from Worker: Signalled properly\n");
				enqueue_cbq(clinic_info->cbq, clinic_info->wq); //TODO: need to store in
			}
			break;
		case ETIMEDOUT: /* our time is up */
			//printf("From Worker: timeout happened\n");
			break;
		default:        /* some error occurred (e.g. we got a Unix signal) */
			break;      /* break this switch, but re-do the while loop.   */
	}
	sem_post(&pool->semaphore);
	pthread_mutex_unlock(&pool->mutex);

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
                        //printf("\tvacant ailment: %d\n", person->pinfo->patient_reg_info.ailment);
                        //printf("\tvacant membership: %d\n", person->pinfo->patient_reg_info.membership);
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
void swap(struct list_head **current )
{
    struct list_head *tmp = *current;

    *current = ( *current )->next;

    tmp->next = ( *current )->next;
    ( *current )->next = tmp;

    ( *current )->prev = tmp->prev;
    tmp->prev = *current;

    if ( tmp->next != NULL )
    {
        tmp->next->prev = tmp;
    }
}

void clear(struct list_head *list )
{
    while ( list->next != NULL )
    {
        struct list *current = list->next;
        list->next = list->next->next;
        free( current );
    }

    list->prev = list->next;
}

int cmp_by_code( const void *a, const void *b )
{
    const Queue *left  = a;
    const Queue *right = b;

    return ( right->pinfo->patient_id < left->pinfo->patient_id ) - ( left->pinfo->patient_id < right->pinfo->patient_id );
}

void sort_queue(struct list_head *list, int cmp( const void *, const void * ) )
{
    if ( list->next != NULL )
    {
        int first_iteration = 1;

        for (Queue **first = &list->next, *sorted = NULL, *last = NULL;
              ( *first )->next != last;
              last = sorted )
        {
            Queue **current = first;
            sorted = ( *first )->next;

            for ( ; ( *current )->next != last; current = &( *current )->next )
            {
                if ( cmp( &( *current )->pinfo, &( *current )->next->person ) > 0 )
                {
                    swap( current );
                    sorted = ( *current )->next;
                }
            }

            if ( first_iteration )
            {
                list->prev = *current;
                first_iteration = 0;
            }
        }
    }
}
#endif
// TODO: low time taking thread need to find
// multiple threads waiting for same vip_request which one will schedule
/*a
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *clinic_info), clinic_info_t *clinic_info)
{
	patients_info_t *pinfo;

	int pos = 0;

	//if vip thread is executing by the time new patient arrives
	
	pinfo = register_details();
	
	pthread_mutex_lock(&pool->mutex);

	if(clinic_info->wq->size == clinic_info->wq->capacity){
		printf("Wait room is Full. Evicting low prios\n");
		bool min_find = find_min(clinic_info->wq, pinfo, &pos); // this requires when you have large buffer
		if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			if(pthread_cond_broadcast(&vip_request) == 0) {
				printf("signal successful from : %s\n", __func__);
				enqueue_front(clinic_info->cbq, pinfo, false);
			} else {
				if(min_find) {
					printf("Found Min element in WQ nodes \n");
					patients_info_t *temp = dequeue_pos(clinic_info, pos); //dq from LL from position
					enqueue(clinic_info->wq, pinfo, true); //enqueue at last
					enqueue(clinic_info->cbq, temp, false); //enqueue at last
				} else
					enqueue(clinic_info->cbq, pinfo, false);
			}
		} else {
			enqueue(clinic_info->cbq, pinfo, false);
		}
	} else {
		enqueue(clinic_info->wq, pinfo, true);
		//sort_queue(&clinic_info->wq->list, cmp_by_code);
	}

	sem_post(&pool->semaphore);
	pthread_mutex_unlock(&pool->mutex);
	printf("pool submitted WQsize: %d CBQSize : %d\n", clinic_info->wq->size, clinic_info->cbq->size);
	return 0;
}

#if 0
//	printf("Enter into pool submit %p : %p WQsize: %d CBQSize : %d\n", testQueue, cbQueue, testQueue->size, cbQueue->size);
//if(running == 1)
pinfo = register_details();
#if 1
if(clinic_info->wq->size == clinic_info->wq->capacity){
	printf("Wait room is Full. Evicting low prios\n");
	if(find_min(clinic_info->wq, intpt_min, pinfo, &pos)) { //Need to delete dispatched thread as min thread
		printf("Min is found %d...! no need to delete : %d\n", pos, intpt_min->find_min);
		printf("Maxpos is found %d...! need to delete ", pos);
		if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			patients_info_t *temp = dequeue_pos(clinic_info->wq, pos);
			printf("value : %d \n", temp->patient_reg_info.membership);
			enqueue(clinic_info->wq, temp, true);
			enqueue(clinic_info->cbQueue, temp, false);
			pthread_cond_signal(&vip_request);
		} else
			enqueue(clinic_info->cbQueue, pinfo, false);
	} else
		enqueue(clinic_info->cbQueue, intpt_min->ptr, false);
} else {
	if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP)
		pthread_cond_signal(&vip_request);
	enqueue(clinic_info->wq, pinfo, true);
}
//		if(running == 1)
#else
if(testQueue->size == testQueue->capacity){
	printf("Wait room is Full. Evicting low prios\n");
	if(find_min(testQueue, intpt_min, pinfo, &pos)) { //Need to delete dispatched thread as min thread
		printf("Min is found %d...! no need to delete : %d\n", pos, intpt_min->find_min);
		enqueue(cbQueue, intpt_min->ptr, false);
	} else {
		printf("Maxpos is found %d...! need to delete ", pos);
		if(pinfo->patient_reg_info.membership == MEMBERSHIP_VIP) {
			patients_info_t *temp = dequeue_pos(testQueue, pos);
			printf("value : %d \n", temp->patient_reg_info.membership);
			enqueue(cbQueue, temp, false);
			pthread_cond_signal(&vip_request);
		} else
			enqueue(cbQueue, pinfo, false);
	}
} else
if(running == 1)
	enqueue(testQueue, pinfo, true);
#endif
	//enqueue(testQueue, pinfo, false);

	sem_post(&pool->semaphore);
	pthread_mutex_unlock(&pool->mutex);
	printf("pool submitted WQsize: %d CBQSize : %d\n", clinic_info->wq->size, clinic_info->cbq->size);

	return 0;
	}
#endif

void pool_init(clinic_info_t *clinicinfo)
{
	int j = 1;
	pool = (struct threadpool*) malloc(sizeof(struct threadpool));
	pool->head = NULL;
	pool->tail = NULL;
	pool->timespan = 10;
	pthread_mutex_init(&pool->mutex, NULL);
	pthread_mutex_init(&pool->cbmutex, NULL);
	sem_init(&pool->semaphore, 0, NUMBER_OF_THREADS);
	begin = time(NULL);
	for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
		pthread_create(&doctors, NULL, worker, (void *) j++);
	}
	printf("created threads successfully\n");
}

void pool_shutdown(void)
{
	pthread_cond_destroy(&vip_request);
	pthread_join(doctors,NULL);
	if (DEBUG) printf("End of execution :)\n");
}
