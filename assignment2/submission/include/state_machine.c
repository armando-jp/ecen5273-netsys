#include <stdio.h>
#include <pthread.h>

#include "state_machine.h"
#include "threading.h"
#include "html.h"
#include "socket.h"

void sm_server()
{
    static state_t current_state = state_idle;
    static event_t current_event = evt_none;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                printf("Server waiting for connection\n");
                if(sock_wait_for_connection()) 
                {
                    current_event = evt_none;
                }
                else
                {
                    current_event = evt_connection;
                }

                if(current_event == evt_connection)
                {
                    current_state = state_creating_thread;
                    current_event = evt_none;
                }

            break;

            case state_creating_thread:
                if(threading_create_dispatcher(sock_get_new_fd()) != 0)
                {
                    current_event = evt_thread_create_fail;
                }
                else
                {
                    current_event = evt_thread_create_success;
                }

                if(current_event ==  evt_thread_create_fail)
                {
                    printf("Error creating dispatcher thread\n");
                    printf("Closing new_fd\n");
                    sock_close(sock_get_new_fd());
                }
                current_state = state_idle;

            break;

            default:
                // something went wrong
                current_state = state_idle;
                current_event = evt_none;
            break;
        }
    }
}

void *sm_dispatch_thread(void *p_args)
{
    static state_t current_state = state_idle;
    static event_t current_event = evt_none;

    thread_args_t args = *(thread_args_t *)p_args;
    char in_buf[100];
    int ret;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                // printf("In dispatcher thread. Waiting for command from client\n");
                printf("Dispatcher %d: waiting for message from fd=%d\n", args.thread_id, args.new_fd);
                // do idle state stuff
                ret = sock_read(args.new_fd, in_buf, 100);
                printf("Dispatcher %d: GOT %d BYTES FROM CLIENT====\n", args.thread_id, ret);
                printf("%s",in_buf);
                printf("Dispatcher %d: ====\n", args.thread_id);

                printf("Dispatcher %d: terminating and closing connection to fd=%d\n", args.thread_id, args.new_fd);
                sock_close(args.new_fd);
                pthread_exit(0);

            break;

            case state_creating_thread:
                // create worker thread
            break;

            default:
                // something went wrong
                current_state = state_idle;
                current_event = evt_none;
            break;
        }
    }
}

void sm_worker_thread()
{
    static state_t current_state = state_idle;
    static event_t current_event = evt_none;

    while(1)
    {
        switch(current_state)
        {
            case state_processing_request:
                // process the request
            break;

            default:
                // something went wrong
                current_state = state_idle;
                current_event = evt_none;
            break;
        }
    }
}