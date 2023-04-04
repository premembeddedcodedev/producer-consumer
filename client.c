#include "client.h"
#include "threadpool.h"
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

static volatile int keepRunning = 1;
static volatile int seq = 0;

clinic_info_t *clinic_info;

void intHandler(int dummy) {
    keepRunning = 0;
}

void print_list_data(struct list_head *program_list)
{
	Queue *p;

	list_for_each_entry(p, program_list, list){
		printf("\tailment: %d\t", p->pinfo->patient_reg_info.ailment);
		printf("\tmembership: %d\n", p->pinfo->patient_reg_info.membership);
	}
}

Queue * initQueue(int max)
{
	Queue *Q;
	Q = (Queue *)malloc(sizeof(Queue));
	Q->size = 0;
	Q->seq = seq;
	Q->capacity = max;
	/* initialize the list head. Kernel list method */
	INIT_LIST_HEAD(&Q->list);
	return Q;
}

Queue *dequeue_ptr(Queue *Q)
{
	Queue* tmp;
	Queue *pinfo;

	if(Q->size==0){
		//printf("Queue is Empty.\n");
		return NULL;
	}else{
		Q->size--;
		tmp = list_entry(Q->list.next, Queue, list);
		pinfo = tmp;
		list_del(Q->list.next);
		free(tmp);
		return pinfo;
	}
}

patients_info_t *dequeue(Queue *Q, bool is_waitq)
{
	patients_info_t *ptr;
	Queue* tmp;

	if(Q->size==0){
		//printf("Queue is Empty.\n");
		return NULL;
	}else{
		Q->size--;
		printf("Deq: #elements in %s are : %d\n", (is_waitq ? "WQ": "CBQ"), Q->size);
		tmp = list_entry(Q->list.next, Queue, list);
		ptr = tmp->pinfo;
		//printf("membership: %d\n", ptr->patient_reg_info.membership);
		list_del(Q->list.next);
		free(tmp);
		//printf("membership: %d\n", ptr->patient_reg_info.membership);
		//print_list_data(&Q->list);
		return ptr;
	}
}

int enqueue(Queue *Q, patients_info_t *pinfo, bool is_waitq)
{
	Queue* newQ;
	if(Q->size == Q->capacity){
		printf(" %s : Queue is Full. No element added.\n", (is_waitq ? "WQ": "CBQ"));
		return -1;
	}
	else{
		Q->size++;
		printf("Enq: #elements in %s : %d\n", (is_waitq ? "WQ": "CBQ"), Q->size);
		newQ = (Queue*) malloc(sizeof(Queue));
		newQ->seq = seq++;
		newQ->pinfo = pinfo;
		/* add to the list tail */
		list_add_tail(&(newQ->list), &(Q->list));
		//print_list_data(&Q->list);
		return 0;
	}
}

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

void cbfunc(void *param)
{
	printf("Enter into %s\n", __func__);
}

void *register_patients(void *param)
{
	int val = 0, check = NUMBER_OF_THREADS;
	srand(time(NULL));

	while(1) {
		val = (rand() % 6) + 4;
		printf("\n\n************ Patient is about to enter after @(%ds) time....\n", val);
		sleep(val);
		pool_submit(cbfunc, clinic_info);
		//printf("WQ Data:\n");
		//print_list_data(&clinic_info->wq->list);
		//printf("CBQ Data:\n");
		//print_list_data(&clinic_info->cbq->list);


		while(check >= 1)  {
			printf("Doctor %d: patients served: %d, interrupter: %d\n", check, clinic_info->dinfo[check].num_patients, clinic_info->dinfo[check].interrupt_count);
			check--;
		}
		check = NUMBER_OF_THREADS;

		printf("****************** Patient left.......\n\n");
	}

	pthread_exit(0);
}

void *process_leftover(void *param)
{
	while(1) {
		process_cbq(clinic_info);
	}

	pthread_exit(0);
}

void *getinstance(void)
{
	return clinic_info;
}

int main()
{
	clinic_info = (clinic_info_t *) malloc (sizeof(clinic_info_t));
	if(!clinic_info) {
		printf("No memory allocated \n");
		return -ENOMEM;
	}

	memset(clinic_info, 0, sizeof(clinic_info_t));

	clinic_info->wq = initQueue(WQMAX_ROOM_SIZE);
	clinic_info->cbq  = initQueue(CBQMAX_ROOM_SIZE);

	printf("main %p : %p\n", clinic_info->wq, clinic_info->cbq);
	pool_init(clinic_info);

	pthread_create(&clinic_info->reception, NULL, register_patients, (void *) clinic_info);
	pthread_create(&clinic_info->leftroom, NULL, process_leftover, (void *) clinic_info);

	//TODO: Create threads here to add the data into below queues
	
	sleep(1);
	pool_shutdown();

	//TODO: [PV] Need to cleanup memory

	return 0;
}
