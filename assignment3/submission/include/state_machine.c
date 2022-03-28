#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "state_machine.h"
#include "threading.h"
#include "http.h"
#include "socket.h"

#define DEBUG_PRINT (0)

void sm_server(int sockfd_listen)
{
    int new_fd;
    state_t current_state = state_idle;
    event_t current_event = evt_none;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                printf("SV: Waiting for connection\n");
                if((new_fd = sock_wait_for_connection(sockfd_listen)) == -1) 
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
                if(threading_create_dispatcher(new_fd) != 0)
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
                    sock_close(sockfd_listen);
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
    // get parameters passed to thread
    thread_args_t args = *(thread_args_t *)p_args;

    // variable to enable timeout
    bool is_timeout = false;
    // variables to store socket reads
    int in_buf_size = 1000;
    char in_buf[in_buf_size];
    int recvbytes;


    // struct to store worker arg
    http_req_results_t *p_worker_args = NULL;

    // worker thread variables
    pthread_t worker_thread_id[MAX_NUMBER_OF_THREADS];
    http_req_results_t worker_args[MAX_NUMBER_OF_THREADS];
    uint8_t worker_thread_counter = 0;

    // state machine variables
    state_t current_state = state_idle;
    event_t current_event = evt_none;

    while(1)
    {
        switch(current_state)
        {
            case state_idle:
                /***************************************************************
                 * (1) Clear input buffer and wait to receive from client.
                 * IF: a failure is encountered while receiving
                 *      IF: timeout was set
                 *          Close connection to client.
                 *      ELSE:
                 *          Unexpected error during recv()
                 * ELIF: payload was 0
                 *      EOT was received. Close connection.
                 * ELSE: 
                 *      Sucessfully received messasge from client. 
                 * 
                 * (2) Progress state machine.
                 **************************************************************/
                memset(in_buf, 0, in_buf_size);
                printf("DP %d: waiting for message from fd=%d\n", args.thread_id, args.fd_client);
                recvbytes = sock_read(args.fd_client, in_buf, in_buf_size, is_timeout);
                if(recvbytes == -1)
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
                else if(recvbytes == 0)
                {
                    current_event = evt_close_request;
                    printf("DP %d: Got EOF from client. Shutting down.\n", args.thread_id);
                }
                else
                {
                    current_event = evt_message_received;
                    printf("DP %d: GOT %d BYTES FROM CLIENT (HTTP REQUEST)\n", args.thread_id, recvbytes);
#if DEBUG_PRINT
                    printf("DP %d: MESSAGE START\n", args.thread_id);
                    printf("%s\r\n", in_buf);
                    http_hex_dump(in_buf, recvbytes);
                    printf("DP %d: MESSAGE END\n", args.thread_id);
#endif
                }

                // check if state machine needs to be progressed.
                if(current_event == evt_message_received)
                {
                    current_state = state_parse_request;
                }
                else if(current_event == evt_close_request || current_event == evt_timeout)
                {
                    printf("DP %d: TERMINATING DISPATCHER THREAD. CLOSING CONNECTION TO CLIENT.\r\n", args.thread_id);
                    sock_close(args.fd_client);
                    pthread_exit(0);
                }

            break;

            
            case state_parse_request:
                /***************************************************************
                 * (1) Parse received HTTP request.
                 * IF: valid HTTP request
                 * ELSE: 
                 * 
                 * (2) Progress state machine.
                 **************************************************************/
                printf("DP %d: Attempting to parse HTTP request.\n", args.thread_id);
                if((p_worker_args = http_parse_request(in_buf, recvbytes)) != NULL)
                {
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
                    p_worker_args->fd_client = args.fd_client;
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
                /***************************************************************
                 * (1) Create the worker thread to handle the HTTP request.
                 * IF: fail to create worker thread
                 * ELSE: worker thread success
                 * 
                 * (2) Check if keep-alive parameter is set.
                 * (3) Progress state machine to IDLE state.
                 **************************************************************/
                // create worker thread
                printf("DP %d: Attempting to create worker thread.\n", args.thread_id);

                // create worker thread params
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS] = *p_worker_args;
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].thread_idx = worker_thread_counter%MAX_NUMBER_OF_THREADS;
                worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS].dp_thread_idx = args.thread_id;

                if(
                    threading_create_worker_proxy(
                        &worker_args[worker_thread_counter%MAX_NUMBER_OF_THREADS], 
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

                // Check if keep_alive parameter is set.
                if(p_worker_args->keep_alive)
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
    http_req_results_t *args = (http_req_results_t *)p_args;

    while(1)
    {
        switch(current_state)
        {
            case state_processing_request:
                /***************************************************************
                 * (1) Generate HTTP response message.
                 * (2) Send HTTP response message.
                 * (3) Terminate worker thread.
                 **************************************************************/

                // DEBUG PRINTS: START /////////////////////////////////////////
                printf(
                    "DP %d WT %d: Printing my params\r\n", 
                    args->dp_thread_idx, 
                    args->thread_idx
                );
                printf("REQ METHOD: %d\r\n", args->req_method);
                printf("HTTP VERSION: %d\r\n", args->req_version);
                printf("REQ URI: %s\r\n", args->p_request_uri);
                printf("CONNECTION HANDLE: %d\r\n", args->fd_client);
                printf(
                    "KEEP ALIVE: %s\r\n", 
                    args->keep_alive  ? "true" : "false"
                );
                if(args->p_request_payload != NULL)
                {
                    printf("PAYLOAD: %s\r\n", args->p_request_payload);
                }
                // DEBUG PRINTS: END ///////////////////////////////////////////

                // (1) Generate HTTP response message.
                int msg_size = 0;
                char *msg = http_create_response(args, &msg_size);
                if(msg == NULL)
                {
                    printf("DT: %d WT %d: Failed to create response MSG.\r\n", args->dp_thread_idx, args->thread_idx);
                }
                else
                {
                    // (2) Send HTTP response message.
                    if (sock_send(args->fd_client, msg, msg_size) == -1)
                    {
                        printf("DT: %d WT %d: Failed to send HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                    else
                    {
                        printf("DT: %d WT %d: Sent HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                }

                // (3) Termiante worker thread.
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
            break;
        }
    }
}

void *sm_worker_thread_proxy(void *p_args)
{
    if(p_args == NULL)
    {
        printf("WT: GOT NULL ARGS! Terminating.\r\n");
    }

    state_t current_state = state_processing_request;
    http_req_results_t *args = (http_req_results_t *)p_args;
    int fd_server;
    int in_buf_size = 40000;
    char in_buf[in_buf_size];
    int numbytes = 0;

    while(1)
    {
        switch(current_state)
        {
            case state_processing_request:
                /***************************************************************
                 * (1) Open new TCP connection to server. 
                 * (2) Send HTTP packet to server.
                 * (3) Wait for HTTP response from server.
                 * (4) Forward HTTP response from server, to client.
                 * (5) Terminate worker thread.
                 **************************************************************/

                // DEBUG PRINTS: START /////////////////////////////////////////
#if DEBUG_PRINT
                printf(
                    "DP %d WT %d: Printing my params\r\n", 
                    args->dp_thread_idx, 
                    args->thread_idx
                );
                printf("\tREQ METHOD: %d\r\n", args->req_method);
                printf("\tHTTP VERSION: %d\r\n", args->req_version);
                printf("\tREQ URI: %s\r\n", args->p_request_uri);
                printf("\tCONNECTION HANDLE: %d\r\n", args->fd_client);
                printf(
                    "\tKEEP ALIVE: %s\r\n", 
                    args->keep_alive  ? "true" : "false"
                );
                if(args->p_request_payload != NULL)
                {
                    printf("\tPAYLOAD: %s\r\n", args->p_request_payload);
                }
                printf("\tORIGINAL HTTP REQUEST: \r\n%s\r\n", args->p_original_http_request);
#endif
                // DEBUG PRINTS: END ///////////////////////////////////////////

                // (1) Open new TCP connection to server. 
                if((fd_server = sock_connect_to_host(args->p_request_host, args->port, args->p_service)) == -1)
                {
                    printf("DT: %d WT %d: Failed to connect to server\r\n", args->dp_thread_idx, args->thread_idx);
                    printf("DT: %d WT %d: TERMINATING\n", args->dp_thread_idx, args->thread_idx);
                    pthread_exit(0);
                }

                // (2) Send HTTP GET request packet to server.
                printf("DT: %d WT %d: Forwarding original HTTP request from client to server\r\n", args->dp_thread_idx, args->thread_idx);
                if (sock_send(fd_server, args->p_original_http_request, args->original_http_request_size) == -1)
                {
                    printf("DT: %d WT %d: Failed to send HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                }
                else
                {
                    printf("DT: %d WT %d: Sent HTTP request to server.\r\n", args->dp_thread_idx, args->thread_idx);
                }

                // (3) Wait for HTTP response from server.
                numbytes = sock_read(fd_server, in_buf, in_buf_size, false);
                if(numbytes == -1)
                {
                    printf("DT: %d WT %d: Unexpected error during recv.\n", args->dp_thread_idx, args->thread_idx);
                }
                else if(numbytes == 0)
                {
                    printf("DT: %d WT %d: Got EOF from SERVER. Shutting down.\n", args->dp_thread_idx, args->thread_idx);
                }
                else
                {
                    printf("DT: %d WT %d: GOT %d BYTES FROM SERVER\n", args->dp_thread_idx, args->thread_idx, numbytes);
#if DEBUG_PRINT
                    printf("DT: %d WT %d: MESSAGE START\n", args->dp_thread_idx, args->thread_idx);
                    printf("%s\r\n", in_buf);
                    printf("DT: %d WT %d: MESSAGE END\n", args->dp_thread_idx, args->thread_idx);
#endif
                }
                // (4) Forward HTTP response from server, to client.
                if(numbytes > 0)
                {
                    if (sock_send(args->fd_client, in_buf, numbytes) == -1)
                    {
                        printf("DT: %d WT %d: Failed to send HTTP msg\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                    else
                    {
                        printf("DT: %d WT %d: Forwarded HTTP response to client.\r\n", args->dp_thread_idx, args->thread_idx);
                    }
                }

                // (5) Termiante worker thread.
                printf("DT: %d WT %d: TERMINATING\n", args->dp_thread_idx, args->thread_idx);
                // if(p_args != NULL)
                //     free(p_args);
                pthread_exit(0);
            break;

            default:
                // something went wrong
                current_state = state_idle;
            break;
        }
    }
}