#ifndef INC_STATE_MACINE_
#define INC_STATE_MACINE_

//typdef enum for possible states
typedef enum state {
    state_idle,
    state_creating_connection,
    state_creating_thread,
    state_processing_request,
    state_parse_request,
    state_create_http_msg,
    state_send_msg_srv,
    state_send_msg_cli,
    state_wait_resp,
} state_t;

// typedef enum for events
typedef enum event {
    evt_connection,
    evt_connection_failed,
    evt_connection_created,
    evt_thread_create_success,
    evt_thread_create_fail,
    evt_invalid_request,
    evt_pending_request,
    evt_timeout,
    evt_close_request,
    evt_exit,
    evt_request_completed,
    evt_request_failed,
    evt_message_received,
    evt_none
} event_t;

void sm_server(int fd_socket);
void *sm_dispatch_thread(void *p_args);
void *sm_worker_thread(void *p_args);
void *sm_worker_thread_proxy(void *p_args);

#endif /*INC_STATE_MACINE_*/