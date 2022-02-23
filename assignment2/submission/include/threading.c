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
static thread_args_t worker_args[MAX_NUMBER_OF_THREADS];
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

int threading_create_worker(int new_fd, char *in_buf, int size)
{
    // start by creating a copy of the payload.
    char *p_payload_copy = (char *) malloc(size);
    if(p_payload_copy == NULL)
    {
        printf("threading: failed to dynamically allocate memory for worker thread.\n");
        return 1;
    }
    memcpy(p_payload_copy, in_buf, size);

    // prepare args struct for worker thread
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].new_fd = new_fd;
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].thread_id = worker_thread_counter%MAX_NUMBER_OF_THREADS;
    worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].p_payload = p_payload_copy;

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