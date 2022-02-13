#ifndef INC_STATE_MACINE_
#define INC_STATE_MACINE_

//typdef enum for possible states
typedef enum state {
    sendCmd_t,
    sendAck_t,
    transmitPayload_t,
    waitAck_t,
    idle_t,
    logFileInfo_t,
    waitPayload_t,
    savePayload_t,
    null_t,
} state_t;

// typedef enum for events
typedef enum event {
    evtAckRecv_t,
    evtAckNotRecv_t,
    evtFileTransComplete_t,
    evtFileTransNotComplete_t,
    evtPayloadReceived_t,
    evtPayloadNotReceived_t,
} event_t;


void sm_client_put();
void sm_server_put();

#endif /*INC_STATE_MACINE_*/