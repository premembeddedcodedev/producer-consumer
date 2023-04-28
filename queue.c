#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "include/common.h"

Queue * initQueue(int max)
{
	Queue *Q;
	Q = (Queue *)malloc(sizeof(Queue));
	Q->size = 0;
	Q->capacity = max;
	/* initialize the list head. Kernel list method */
	INIT_LIST_HEAD(&Q->list);
	return Q;
}

int enqueue_front(Queue *Q, patients_info_t *pinfo, bool is_waitq)
{
	Queue* newQ;
	if(Q->size == Q->capacity){
		printf(" %s : Queue is Full. No element added.\n", (is_waitq ? "WQ": "CBQ"));
		return -ENOMEM;
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
		newQ->pinfo = pinfo;
		/* add to the list tail */
		list_add_tail(&(newQ->list), &(Q->list));
		//print_list_data(&Q->list);
		return 0;
	}
}

int enqueue_dinfo(Queue *Q, doctor_info_t *pinfo, bool is_waitq)
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
		newQ->dinfo = pinfo;
		/* add to the list tail */
		list_add_tail(&(newQ->list), &(Q->list));
		//print_list_data(&Q->list);
		return 0;
	}
}
