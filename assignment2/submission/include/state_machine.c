#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "state_machine.h"
#include "threading.h"
#include "http.h"
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
    http_req_results_t *p_results = NULL;
    int in_buf_size = 1000;
    char in_buf[in_buf_size];
    int payload_size;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:

                // do idle state stuff
                printf("Dispatcher %d: waiting for message from fd=%d\n", args.thread_id, args.new_fd);

                payload_size = sock_read(args.new_fd, in_buf, in_buf_size, true);
                if(payload_size == -1)
                {
                    current_event = evt_timeout;
                }
                else if (payload_size == 1)
                {
                    printf("Dispatcher %d: error while receiving from client\n", args.thread_id);
                }
                else
                {
                    current_event = evt_message_received;
                    printf("Dispatcher %d: GOT %d BYTES FROM CLIENT====\n", args.thread_id, payload_size);
                    printf("%s",in_buf);
                    printf("Dispatcher %d: ====\n", args.thread_id);
                }

                // check if state machine needs to be progressed.
                if(current_event == evt_timeout)
                {
                    printf("Dispatcher %d: TIMEOUT! Closing connection to client.\n", args.thread_id);
                    sock_close(args.new_fd);
                    pthread_exit(0);
                }
                else if(current_event == evt_message_received)
                {
                    current_state = state_parse_request;
                }

            break;

            case state_parse_request:
                // determine if the request is a valid HTTP request.
                if((p_results = http_parse_request(in_buf, payload_size)) != NULL)
                {
                    args.request = p_results;
                    current_event = evt_pending_request;
                } 
                else
                {
                    // invalid http request received
                    current_event = evt_invalid_request;
                }   

                // process events
                if(current_event == evt_pending_request)
                {
                    // a valid request was received. create a worker thread
                    p_results->fd_connection = args.new_fd;
                    current_state = state_creating_thread;
                }
                else if(current_event == evt_invalid_request)
                {
                    // invalid request was received. clear the input buffer 
                    // and prepare for next transmission.
                    printf("Dispatcher %d: Invalid request received.\n", args.thread_id);
                    current_state = state_idle;
                }

            break;

            case state_creating_thread:
                // create worker thread
                if(threading_create_worker(p_results) != 0)
                {
                    printf("Dispatch %d: Error creating worker thread\n", args.new_fd);
                    current_event = evt_thread_create_fail;
                }
                else
                {
                    printf("Dispatch %d: Success creating worker thread\n", args.new_fd);
                    current_event = evt_thread_create_success;
                }

                // TODO: VVV
                // check if the keep-alive parameter is set.
                // if it is, then continue to the idle state.
                // otherwise, close this dispatcher thread.
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

void *sm_worker_thread(void *p_args)
{
    static state_t current_state = state_processing_request;
    static event_t current_event = evt_none;

    http_req_results_t *args = (http_req_results_t *)p_args;

    while(1)
    {
        switch(current_state)
        {
            case state_processing_request:
                // process the request
                printf("WT: Printing my payload===\n");
                
                printf("CONNECTION HANDLE: %d\r\n", args->fd_connection);
                printf("KEEP ALIVE: %d\r\n", args->keep_alive);
                printf("REQ METHOD: %d\r\n", args->req_method);
                printf("HTTP VERSION: %d\r\n", args->req_version);
                printf("REQ URI: %s\r\n", args->p_request_uri);
                printf("CONTENT LENGTH: %d\r\n", args->content_length);
                printf("PAYLOAD: %s\r\n", args->p_request_payload);

                printf("===\n");

                // send a dummy message
                // CALL FUNCTION TO CREATE BUFFER W/ACTUAL MESSAGE CONTENTS
                char *msg = "HTTP/1.1 200 GOOD\\r\\Content-Length: 92\\r\\n\\r\\n<!DOCTYPE html><html><body><h1>My First Heading</h1><p>My first paragraph.</p></body></html>";
                sock_send(args->fd_connection, msg, strlen(msg));

                // termiante
                printf("WT: terminating\n");
                free(args);
                pthread_exit(0);
            break;

            default:
                // something went wrong
                current_state = state_idle;
                current_event = evt_none;
            break;
        }
    }
}