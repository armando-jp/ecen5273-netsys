#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

int threading_create_worker(http_req_results_t *p_results, pthread_t *id)
{
    int ret = 0;
    if(p_results == NULL)
    {
        printf("threading: invalid parameter\r\n");
        return 1;
    }


    // create the worker thread
    if((ret = pthread_create(
        id, 
        NULL, 
        sm_worker_thread,
        (void*) p_results
    )) != 0)
    {
        if(ret == EAGAIN)
        {
           printf("threading create: Insufficient resources to create another thread.\n"); 
        }
        else if(ret == EINVAL)
        {
            printf("threading create: Invalid settings in attr.\n"); 
        }
        else if(ret == EPERM)
        {
            printf("threading create: No permission to set the scheduling policy and parameters specified in attr.\n"); 
        }
        else
        {
            printf("threading create: error creating worker thread. CODE: %d\n", ret);
        }
        return 1;
    }
    // detach the worker thread
    else if((ret = pthread_detach(*id)) != 0)
    {
        if(ret == EINVAL)
        {
           printf("threading detach: thread is not a joinable thread.\n"); 
        }
        else if(ret == ESRCH  )
        {
            printf("threading detach: No thread with the ID (%ld) thread could be found.\n", *id); 
        }
        else
        {
            printf("threading detach: error detaching worker thread. CODE: %d\n", ret);
        }
        return 1;
    }

    return 0;
}

int threading_create_worker_proxy(http_req_results_t *p_results, pthread_t *id)
{
    int ret = 0;
    if(p_results == NULL)
    {
        printf("threading: invalid parameter\r\n");
        return 1;
    }


    // create the worker thread
    if((ret = pthread_create(
        id, 
        NULL, 
        sm_worker_thread_proxy,
        (void*) p_results
    )) != 0)
    {
        if(ret == EAGAIN)
        {
           printf("threading create: Insufficient resources to create another thread.\n"); 
        }
        else if(ret == EINVAL)
        {
            printf("threading create: Invalid settings in attr.\n"); 
        }
        else if(ret == EPERM)
        {
            printf("threading create: No permission to set the scheduling policy and parameters specified in attr.\n"); 
        }
        else
        {
            printf("threading create: error creating worker thread. CODE: %d\n", ret);
        }
        return 1;
    }
    // detach the worker thread
    else if((ret = pthread_detach(*id)) != 0)
    {
        if(ret == EINVAL)
        {
           printf("threading detach: thread is not a joinable thread.\n"); 
        }
        else if(ret == ESRCH  )
        {
            printf("threading detach: No thread with the ID (%ld) thread could be found.\n", *id); 
        }
        else
        {
            printf("threading detach: error detaching worker thread. CODE: %d\n", ret);
        }
        return 1;
    }

    return 0;
}