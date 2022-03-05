#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "threading.h"
#include "state_machine.h"

/*******************************************************************************
 * Important variables
*******************************************************************************/
static pthread_t dispatcher_thread_id[MAX_NUMBER_OF_THREADS];
static thread_args_t dispatcher_args[MAX_NUMBER_OF_THREADS];
static uint8_t dispatcher_thread_counter = 0;

static pthread_t worker_thread_id[MAX_NUMBER_OF_THREADS];
static http_req_results_t worker_args[MAX_NUMBER_OF_THREADS];
static uint8_t worker_thread_counter = 0;

/*******************************************************************************
 * Thread creating functions
*******************************************************************************/
int threading_create_dispatcher(int new_fd)
{
    // prepare args struct for dispatcher thread
    dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS].new_fd = new_fd;
    dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS].thread_id = dispatcher_thread_counter%MAX_NUMBER_OF_THREADS;
    dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS].p_payload = NULL;

    // create dispatcher thread
    if(pthread_create(
        &dispatcher_thread_id[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS], 
        NULL, 
        sm_dispatch_thread,
        (void*) &dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS]
    ) != 0)
    {
        printf("threading: error creating dispatcher thread\n");
        return 1;
    }

    if(pthread_detach(dispatcher_thread_id[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS]) != 0)
    {
        printf("threading: error detaching dispatcher thread\n");
        return 1;
    }

    dispatcher_thread_counter++;

    return 0;
}

int threading_create_worker(http_req_results_t *p_results, int dp_thread_idx)
{
    if(p_results == NULL)
    {
        printf("threading: invalid parameter\r\n");
        return 1;
    }
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS] = *p_results;
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].thread_idx = worker_thread_counter;
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].dp_thread_idx = dp_thread_idx;

    // create the worker thread
    if(pthread_create(
        &worker_thread_id[worker_thread_counter%MAX_NUMBER_OF_THREADS], 
        NULL, 
        sm_worker_thread,
        (void*) &worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS]
    ) != 0)
    {
        printf("threading: error creating worker thread\n");
        return 1;
    }

    // detach the worker thread
    if(pthread_detach(worker_thread_id[worker_thread_counter%MAX_NUMBER_OF_THREADS]) != 0)
    {
        printf("threading: error detaching worker thread\n");
        return 1;
    }

    worker_thread_counter++;

    return 0;
}