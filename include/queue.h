#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "list.h"
#include "clinic.h"
#include "doctor.h"
#include "patient.h"
#include "common.h"

#ifndef QueueElement
#define QueueElement void*
#endif

typedef struct{
    int seq;
    int capacity;
    int size;
    int pos;
    QueueElement e;
    patients_info_t *pinfo;
    struct list_head list;
} Queue;

int enqueue(Queue *Q, patients_info_t *pinfo, bool is_waitq);
patients_info_t *dequeue(Queue *Q, bool is_waitq);
Queue *dequeue_ptr(Queue *Q);
int enqueue_front(Queue *Q, patients_info_t *pinfo, bool is_waitq);
Queue * initQueue(int max);

#endif
