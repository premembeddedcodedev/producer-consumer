#include <semaphore.h>
#include "client.h"
// function prototypes
void execute(void (*somefunction)(void *p), void *p);
int pool_submit(void (*somefunction)(void *p), clinic_info_t *clinic_info);
void *worker(void *param);
void pool_init(clinic_info_t *clinic_info);
void pool_shutdown(void);

struct task
{
    void (*function)(void *p);
    void *data;
    struct task *next;
    int task_id; //debug purpose
};

struct threadpool
{
    struct task *head;
    struct task *tail;
    pthread_mutex_t mutex;
    pthread_mutex_t cbmutex;
    sem_t semaphore;
    int timespan;
};
