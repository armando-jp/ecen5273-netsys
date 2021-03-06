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
    state_t current_state = state_idle;
    event_t current_event = evt_none;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                printf("SV: Waiting for connection\n");
                if(sock_wait_for_connection()) 
                {
                    current_event = evt_none;
                }
                else
                {
                    printf("SV: got connection\r\n");
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
                    printf("SV: Error creating dispatcher thread\n");
                    printf("SV: Closing new_fd\n");
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
    // state machine variables
    state_t current_state = state_idle;
    event_t current_event = evt_none;

    // get parameters passed to thread
    thread_args_t args = *(thread_args_t *)p_args;

    // struct to store threads for workers
    http_req_results_t *p_results = NULL;

    // variables to store socket reads
    int in_buf_size = 1000;
    char in_buf[in_buf_size];
    int payload_size;

    // variable to enable timeout
    bool is_timeout = false;

    // worker thread variables
    pthread_t worker_thread_id[MAX_NUMBER_OF_THREADS];
    http_req_results_t worker_args[MAX_NUMBER_OF_THREADS];
    uint8_t worker_thread_counter = 0;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                // clear input buffer and wait to receive from client
                memset(in_buf, 0, in_buf_size);
                printf("DP %d: waiting for message from fd=%d\n", args.thread_id, args.new_fd);
                payload_size = sock_read(args.new_fd, in_buf, in_buf_size, is_timeout);
                if(payload_size == -1)
                {
                    if(is_timeout)
                    {
                        printf("DP %d: TIMEOUT! Closing connection to client.\n", args.thread_id);
                        current_event = evt_timeout;
                    }
                    else
                    {
                        printf("DP %d: Unexpected error during recv.\n", args.thread_id);
                    }
                }
                else if(payload_size == 0)
                {
                    current_event = evt_close_request;
                    printf("DP %d: Got EOF from client. Shutting down.\n", args.thread_id);
                }
                else
                {
                    current_event = evt_message_received;
                    printf("DP %d: GOT %d BYTES FROM CLIENT\n", args.thread_id, payload_size);
                    printf("DP %d: MESSAGE START\n", args.thread_id);
                    printf("%s\r\n", in_buf);
                    // for(int i = 0; i < payload_size; i++)
                    // {
                    //     printf("%c", in_buf[i]);
                    // }
                    // printf("\r\n");
                    // http_hex_dump(in_buf, payload_size);
                    printf("DP %d: MESSAGE END\n", args.thread_id);
                }

                // check if state machine needs to be progressed.
                if(current_event == evt_message_received)
                {
                    current_state = state_parse_request;
                }
                else if(current_event == evt_close_request || current_event == evt_timeout)
                {
                    printf("DP %d: TERMINATING\n", args.thread_id);
                    sock_close(args.new_fd);
                    pthread_exit(0);
                }

            break;

            case state_parse_request:
                // determine if the request is a valid HTTP request.
                printf("DP %d: Attempting to parse HTTP request.\n", args.thread_id);
                if((p_results = http_parse_request(in_buf, payload_size)) != NULL)
                {
                    // args.request = p_results;
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
                    printf("DP %d: Received invalid request or failed to parse.\r\n", args.thread_id);
                    current_state = state_idle;
                }
            break;

            case state_creating_thread:
                // create worker thread
                printf("DP %d: Attempting to create worker thread.\n", args.thread_id);

                // create worker thread params
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS] = *p_results;
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].thread_idx = worker_thread_counter%MAX_NUMBER_OF_THREADS;
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].dp_thread_idx = args.thread_id;

                if(
                    threading_create_worker(&worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS], 
                        &worker_thread_id[worker_thread_counter%MAX_NUMBER_OF_THREADS]
                    ) 
                != 0)
                {
                    printf("DP %d: Error creating worker thread\n", args.thread_id);
                    current_event = evt_thread_create_fail;
                }
                else
                {
                    printf("DP %d: Success creating worker thread\n", args.thread_id);
                    worker_thread_counter++;
                    current_event = evt_thread_create_success;
                }

                // TODO: VVV
                // check if the keep-alive parameter is set.
                // if it is, then continue to the idle state.
                // otherwise, close this dispatcher thread.
                if(p_results->keep_alive)
                {
                    printf("DP %d: Detected keep-alive argument. Setting timer\n", args.thread_id);
                    is_timeout = true;
                }
                else
                {
                    printf("DP %d: Did not detect keep-alive.\n", args.thread_id);
                    // sock_close(args.new_fd);
                    // pthread_exit(0);
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

void *sm_worker_thread(void *p_args)
{
    if(p_args == NULL)
    {
        printf("WT: GOT NULL ARGS! Terminating.\r\n");
    }
    state_t current_state = state_processing_request;
    // static event_t current_event = evt_none;

    http_req_results_t *args = (http_req_results_t *)p_args;

    while(1)
    {
        switch(current_state)
        {
            case state_processing_request:
                // process the request
                printf("DP %d WT %d: Printing my params\r\n", args->dp_thread_idx, args->thread_idx);
                printf("REQ METHOD: %d\r\n", args->req_method);
                printf("HTTP VERSION: %d\r\n", args->req_version);
                printf("REQ URI: %s\r\n", args->p_request_uri);
                printf("CONNECTION HANDLE: %d\r\n", args->fd_connection);
                printf("KEEP ALIVE: %s\r\n", args->keep_alive  ? "true" : "false");
                if(args->p_request_payload != NULL)
                {
                    printf("PAYLOAD: %s\r\n", args->p_request_payload);
                }

                // CALL FUNCTION TO CREATE BUFFER W/ACTUAL MESSAGE CONTENTS
                // printf("DT: %d WT %d: CREATING HTTP Response msg\r\n", args->dp_thread_idx, args->thread_idx);
                int msg_size = 0;
                char *msg = http_create_response(args, &msg_size);
                if(msg == NULL)
                {
                    printf("DT: %d WT %d: Failed to create response MSG.\r\n", args->dp_thread_idx, args->thread_idx);
                }
                else
                {
                    // printf("DT: %d WT %d: SENDING HTTP Response msg of size %d bytes\r\n", args->dp_thread_idx, args->thread_idx, msg_size);
                    //http_hex_dump(msg, msg_size);
                    if (sock_send(args->fd_connection, msg, msg_size) == -1)
                    {
                        printf("DT: %d WT %d: Failed to send HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                    else
                    {
                        printf("DT: %d WT %d: Sent HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                }

                // termiante
                printf("DT: %d WT %d: TERMINATING\n", args->dp_thread_idx, args->thread_idx);

                if(msg != NULL)
                {
                    
                    free(msg);
                }
                
                pthread_exit(0);
            break;

            default:
                // something went wrong
                current_state = state_idle;
                // current_event = evt_none;
            break;
        }
    }
}