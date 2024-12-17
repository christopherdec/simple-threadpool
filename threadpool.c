/*
 * Simple threadpool monitor implementation.
 * Created by christopherdec on 24/11/24.
 */


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "threadpool.h"

struct threadpool_job {
    void (* func) (int);
    int arg;
};

static int initialized = 0;
static int shutdown_running = 0;
static int available = 0;

static int pool_size;
static int buffer_size;
static struct threadpool_job **buffer;
static int next_insert;
static int next_remove;

static pthread_t *pool;
static int *worker_ids;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t job_available_cond = PTHREAD_COND_INITIALIZER;


static void worker_cleanup(int *id) {
    printf("worker-%d: Executing cleanup func\n", *id);
    pthread_mutex_unlock(&mutex);
}

/*
 * The worker code is fairly straightforward: 
 * If no job is available, the thread is blocked until job_available_cond is signaled.
 * The thread may be canceled while waiting for job_available_cond on when testing cancel. 
 * In any case, the cleanup function should be executed to unlock the mutex.
 */
static void worker(int *id) {
    struct threadpool_job *job;

    printf("worker-%d: Initializing\n", *id);

    pthread_cleanup_push((void *) worker_cleanup, (void *) id);

    while(1) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        pthread_mutex_lock(&mutex);
        while(!available) {
            printf("worker-%d: No job available, waiting for cond\n", *id);
            pthread_cond_wait(&job_available_cond, &mutex);
        }
        pthread_testcancel();
        job = buffer[next_remove];
        next_remove = (next_remove + 1) % buffer_size;
        available--;
        pthread_mutex_unlock(&mutex);
    
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        job->func(job->arg);
        free(job);
    }

    pthread_cleanup_pop(0);
}


int threadpool_init(int _pool_size, int _buffer_size) {
    int ret;

    pthread_mutex_lock(&mutex);
    if (initialized || shutdown_running)
        ret = -1;
    else {
      	printf("threadpool: Initializing with pool_size=%d, buffer_size=%d\n", _pool_size, _buffer_size);
        pool_size = _pool_size;
        buffer_size = _buffer_size;
        if ((buffer = calloc(buffer_size, sizeof(struct threadpool_job *))) == NULL) {
            ret = -1;
        } else if ((pool = calloc(pool_size, sizeof(pthread_t))) == NULL) {
            ret = -1;
            free(buffer);
        } else if ((worker_ids = calloc(pool_size, sizeof(int))) == NULL) {
            ret = -1;
            free(buffer);
            free(pool);
        }
        else /* memory allocation success */ {
            ret = 0;
            initialized = 1;
            next_insert = 0;
            next_remove = 0;

            for (int i = 0; i < pool_size; i++) {
                worker_ids[i] = i;
                pthread_create(&pool[i], NULL, (void *) worker, &worker_ids[i]);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    return ret;
}


int threadpool_submit(void(* func)(int), int arg) {
    if (initialized == 0 || shutdown_running)
        return -1;

    struct threadpool_job *job = malloc(sizeof(struct threadpool_job));

    if (job == NULL)
        return -1;

    job->func = func;
    job->arg = arg;

    int ret;

    pthread_mutex_lock(&mutex);
    if (available == buffer_size)
        ret = -1;
    else {
        ret = 0;
        buffer[next_insert] = job;
        printf("threadpool: Received job %d\n", next_insert);
        next_insert = (next_insert + 1) % buffer_size;
        available++;
        pthread_cond_signal(&job_available_cond);
    }
    pthread_mutex_unlock(&mutex);

    if (ret == -1)
        free(job);

    return ret;
}

int threadpool_queue_size(void) {
    int ret;
    pthread_mutex_lock(&mutex);
    ret = available;
    pthread_mutex_unlock(&mutex);
    return ret;
}

int threadpool_shutdown(void) {
    pthread_mutex_lock(&mutex);
    if (initialized == 0 || shutdown_running)
        return -1;

    shutdown_running = 1;
    printf("threadpool: Shutdown started\n");

    for (int i = 0; i < pool_size; i++) {
        printf("threadpool: Attempting to cancel worker-%d\n", i);
        pthread_cancel(pool[i]);
    }
    pthread_mutex_unlock(&mutex);
    
    for (int i = 0; i < pool_size; i++) {
        printf("threadpool: Attempting to join worker-%d\n", i);
        pthread_join(pool[i], NULL);
    }

    struct threadpool_job *job;
    while(available > 0) {
        job = buffer[next_remove];
        free(job);
        next_remove = (next_remove + 1) % buffer_size;
        available--;
    }

    free(buffer);
    free(pool);
    free(worker_ids);

    pthread_mutex_lock(&mutex);
    available = 0;
    initialized = 0;
    shutdown_running = 0;
    printf("threadpool: Shutdown finished\n");
    pthread_mutex_unlock(&mutex);

    return 0;
}
