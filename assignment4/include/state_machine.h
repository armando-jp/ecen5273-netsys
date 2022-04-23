#ifndef INC_STATE_MACINE_
#define INC_STATE_MACINE_

#include "packet.h"
#include "conf_parsing.h"

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

typedef struct {
    int dfs1;
    int dfs2;
    int dfs3;
    int dfs4;
} fd_dfs_t;

/*******************************************************************************
 * DFS state machines
 ******************************************************************************/
void sm_server(int sockfd_listen);
void *sm_server_thread(void *p_args);

/*******************************************************************************
 * DFC state machines
 ******************************************************************************/
void sm_get(char *file_name, conf_results_t conf, fd_dfs_t fd);
void sm_send(char *file_name, conf_results_t conf, int fd);

/*******************************************************************************
 ******************************************************************************/
void sm_receive(int fd, Packet pkt, int is_server);
int sm_receive_pieces(int fd);

#endif /*INC_STATE_MACINE_*/