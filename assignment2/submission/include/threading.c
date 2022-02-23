#include <pthread.h>
#include <stdio.h>

#include "threading.h"
#include "state_machine.h"

/*******************************************************************************
 * Important variables
*******************************************************************************/
static pthread_t dispatcher_thread_id[MAX_NUMBER_OF_THREADS];
static thread_args_t dispatcher_args[MAX_NUMBER_OF_THREADS];
static uint8_t dispatcher_thread_counter = 0;

/*******************************************************************************
 * Thread creating functions
*******************************************************************************/
int threading_create_dispatcher(int new_fd)
{
    // prepare args struct for dispatcher thread
    dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS].new_fd = new_fd;
    dispatcher_args[dispatcher_thread_counter%MAX_NUMBER_OF_THREADS].thread_id = dispatcher_thread_counter%MAX_NUMBER_OF_THREADS;

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