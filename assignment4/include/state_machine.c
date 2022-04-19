#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "state_machine.h"
#include "threading.h"
#include "packet.h"
#include "socket.h"
#include "conf_parsing.h"
#include "file.h"
#include "crc.h"

#define DEBUG_PRINT_DISPATCHER (0)
#define DEBUG_PRINT_WORKER     (0)

extern char *dfs_name;

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
                // wait for incomming connection
                printf("%s: Waiting for connection\r\n", dfs_name);
                if((new_fd = sock_wait_for_connection(sockfd_listen)) == -1) 
                {
                    current_event = evt_none;
                }
                else
                {
                    printf("%s: got connection\r\n", dfs_name);
                    current_event = evt_connection;
                }

                // progress state machine
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

    // variables to store socket read results
    char in_buf[PACKET_SIZE];
    int recvbytes;

    // // worker thread variables
    // pthread_t worker_thread_id[MAX_NUMBER_OF_THREADS];
    // worker_thread_args_t worker_args[MAX_NUMBER_OF_THREADS];
    // uint8_t worker_thread_counter = 0;

    // // state machine variables
    // state_t current_state = state_idle;
    // event_t current_event = evt_none;

    Packet pkt;

    while(1)
    {
        // 1. Get an incomming message.
        memset(in_buf, 0, PACKET_SIZE);
        printf("%s#%d: waiting for message from client @fd=%d\n", dfs_name, args.thread_id, args.fd_client);
        recvbytes = sock_read(args.fd_client, in_buf, PACKET_SIZE, is_timeout);
        if(recvbytes == -1)
        {
            if(is_timeout)
            {
                printf("%s#%d: TIMEOUT! Closing connection to client.\n", dfs_name, args.thread_id);
                // current_event = evt_timeout;
            }
            else
            {
                printf("%s#%d: Unexpected error during recv.\n", dfs_name, args.thread_id);
            }
        }
        else if(recvbytes == 0)
        {
            // current_event = evt_close_request;
            printf("%s#%d: Got EOF from client. Shutting down.\n", dfs_name, args.thread_id);
        }
        else
        {
            // current_event = evt_message_received;
            printf("%s#%d: GOT %d BYTES FROM CLIENT (DFC COMMAND)\n", dfs_name, args.thread_id, recvbytes);

        }

        // 2. Parse received command
        printf("%s#%d: Attempting to parse DFC command.\n", dfs_name, args.thread_id);
        pkt = packet_parse_packet(in_buf, recvbytes);
        if(packet_is_default(pkt))
        {
            // invalid request received
            printf("%s#%d: received invalid request", dfs_name, args.thread_id);
            continue;
        }  

        // 3. process command
        // (0) Debug print pkt
        packet_print(pkt);

        // (1) Process request
        switch(pkt.cmd_header)
        {
            case CMD_GET_PKT:
                printf("%s#%d: Made it to GET cmd\r\n", dfs_name, args.thread_id);
                // TODO: ARMANDO
                // make up a conf variable just for sending
                // sm_send(pkt.file_name, conf, args.fd_client) to client.
            break;

            case CMD_PUT_PKT:
                printf("%s#%d: Made it to PUT cmd\r\n", dfs_name, args.thread_id);
                sm_receive(args.fd_client, pkt);

            break;

            case CMD_LS_PKT:
                printf("%s#%d: Made it to LS cmd\r\n", dfs_name, args.thread_id);
            break;

            default:
                printf("%s#%d: Invalid CMD received: %d\r\n", dfs_name, args.thread_id, pkt.cmd_header);
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

    worker_thread_args_t *args = (worker_thread_args_t *)p_args;

    /***************************************************************
     * (0) Debug print pkt
     * (1) Respond to packet
     * (2) Terminate worker thread
     **************************************************************/

    // (0) Debug print pkt
    packet_print(args->pkt);

    // (1) Process request
    switch(args->pkt.cmd_header)
    {
        case CMD_GET_PKT:
            printf("Made it to GET cmd\r\n");
        break;

        case CMD_PUT_PKT:
            printf("Made it to PUT cmd\r\n");
            sm_receive(args->fd_client, args->pkt);

        break;

        case CMD_LS_PKT:
            printf("Made it to LS cmd\r\n");
        break;

        default:
            printf("Invalid CMD received: %d\r\n", args->pkt.cmd_header);
        break;
    }
                
    // (2) Termiante worker thread.
    printf("Terminating\r\n");
    pthread_exit(0);

}

/*******************************************************************************
 * DFC state machines
 ******************************************************************************/
// This function sends a GET request for `file_name` to the socket connection 
// `fd` using user_name and password from `conf`.
void sm_get(char *file_name, conf_results_t conf, fd_dfs_t fd)
{
    // TODO: ARMANDO
    // 1. Create pkt containing file_name and GET cmd + other essential
    //    variables from `conf`.
    // 2. Send the command to `fd`
    // 3. Wait for response tcp message.
    // 4. Enter sm_receive() to obtain entierty of file.
    uint32_t bytes_sent = 0;
    uint32_t packet_size = 0;
    char out_buffer[PACKET_SIZE];

    // Generate the CMD header
    Packet pkt = packet_get_default();

    strcpy(pkt.user_name, "Amy");
    strcpy(pkt.password, "KoolPass");
    pkt.cmd_header = CMD_GET_PKT;
    pkt.crc32_header = 1198457;
    pkt.payload_header = 100;
    pkt.payload[0] = '1';
    pkt.payload[pkt.payload_header-1] = '1';

    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet
    bytes_sent = sock_send(fd_dfs1, out_buffer, packet_size);

    printf("packet_size: %d, bytes_sent: %d\r\n", packet_size, bytes_sent);

    return;
}

// This function will send the `file_name` to socket connection `fd` with 
// user_name and passowrd from `conf`.
void sm_send(char *file_name, conf_results_t conf, fd_dfs_t fd)
{
    FILE *file;
    uint32_t file_bytes_read;
    uint32_t file_bytes_sent;
    uint32_t out_buffer_size;
    uint32_t bytes_sent;
    char out_buffer[PACKET_SIZE];
    char file_buffer[PAYLOAD_CHUNK_SIZE];

    memset(out_buffer, 0, PACKET_SIZE);
    memset(file_buffer, 0, PAYLOAD_CHUNK_SIZE);

    // 1. Attempt to open the specified file.
    file = file_open(file_name, 0);
    if(file == NULL)
    {
        printf("%s does not exist\r\n", file_name);
        return;
    }

    // 2. Generate initial packet
    Packet pkt = packet_get_default();
    strcpy(pkt.user_name, conf.user_name);
    strcpy(pkt.password, conf.password);
    pkt.cmd_header = CMD_PUT_PKT;
    pkt.crc32_header = crc_generate_file(file);
    strcpy(pkt.file_name, file_name);
    pkt.payload_header = file_get_size(file);

    // 2.5: Print pkt
    packet_print(pkt);

    // 4. Send packets
    if(pkt.payload_header <= PAYLOAD_CHUNK_SIZE)
    {
        printf("The file will fit in one transfer\r\n");
        // 5. Copy payload
        file_read(file_buffer, file, pkt.payload_header);
        memcpy(pkt.payload, file_buffer, pkt.payload_header);

        // 6. Turn packet into a bytes buffer
        out_buffer_size = packet_convert_to_buffer(pkt, out_buffer);

        // 7. Send initial packet
        bytes_sent = sock_send(fd.dfs1, out_buffer, out_buffer_size);
        printf("Sent %d bytes out of %d bytes to DFS1\r\n", bytes_sent, out_buffer_size);
    }  
    else
    {
         printf("The file requires multiple transfers\r\n");

        // 5. Copy payload
        file_bytes_read = file_read(file_buffer, file, PAYLOAD_CHUNK_SIZE);
        printf("file_bytes_read: %d\r\n", file_bytes_read);
        memcpy(pkt.payload, file_buffer, PAYLOAD_CHUNK_SIZE);
        // 6. Turn packet into a bytes buffer
        out_buffer_size = packet_convert_to_buffer(pkt, out_buffer);
        printf("out_buffer_size: %d\r\n", out_buffer_size);
        
        // 7. Send initial packet
        bytes_sent = sock_send(fd.dfs1, out_buffer, out_buffer_size);

        // 8. Send the rest of the payload
        file_bytes_sent = PAYLOAD_CHUNK_SIZE;
        printf("Sent %d/%d bytes.\r\n", file_bytes_sent, pkt.payload_header);
        while(file_bytes_sent < pkt.payload_header)
        {
            // 9. Clear buffers
            memset(out_buffer, 0, PACKET_SIZE);
            memset(file_buffer, 0, PAYLOAD_CHUNK_SIZE);

            // 10. Send a pure payload
            if((pkt.payload_header - file_bytes_sent) < PAYLOAD_CHUNK_SIZE)
            {
                // 11. Copy payload
                file_read(file_buffer, file, (pkt.payload_header - file_bytes_sent));

                // 12. Send the pure payload
                bytes_sent = sock_send(fd.dfs1, file_buffer, (pkt.payload_header - file_bytes_sent));
                
                file_bytes_sent += (pkt.payload_header - file_bytes_sent);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
            else
            {
                file_read(file_buffer, file, PAYLOAD_CHUNK_SIZE);
                file_bytes_sent += PAYLOAD_CHUNK_SIZE;

                bytes_sent = sock_send(fd.dfs1, file_buffer, PAYLOAD_CHUNK_SIZE);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
        }
    }

    return;
}

// This function will receive a file from socket connection `fd`. The initial 
// parameters of the file are provided to the function in `pkt`.
void sm_receive(int fd, Packet pkt)
{
    FILE *file;
    uint32_t bytes_saved = 0;
    uint32_t bytes_recv = 0;
    char in_buf[PAYLOAD_CHUNK_SIZE];
    uint32_t file_bytes_recv = 0;

    // Check if we got the size of the payload
    if(pkt.payload_header == 0)
    {
        printf("No payload noted in payload_header\r\n");
        return;
    }

    char file_path[FILE_NAME_SIZE + 21];
    sprintf(file_path, "./%s/%s", dfs_name, pkt.file_name);

    // Open/create file
    file = file_open_create(file_path, 1);
    if(file == NULL)
    {
        printf("Failed to create file %s\r\n", file_path);
        return;
    }


    if(pkt.payload_header > PAYLOAD_CHUNK_SIZE)
    {
        file_bytes_recv = PAYLOAD_CHUNK_SIZE;

        // Write initial payload
        bytes_saved = file_write(pkt.payload, file, PAYLOAD_CHUNK_SIZE);
        printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);
        file_bytes_recv = PAYLOAD_CHUNK_SIZE;
        while(file_bytes_recv < pkt.payload_header)
        {
            memset(in_buf, 0, PAYLOAD_CHUNK_SIZE);
            bytes_recv = sock_read(fd, in_buf, PAYLOAD_CHUNK_SIZE, 0);
            bytes_saved += file_write(in_buf, file, bytes_recv);
            printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);
            file_bytes_recv += bytes_recv;
        }
    }
    else
    {
        // Write initial payload
        bytes_saved = file_write((char *)pkt.payload, file, pkt.payload_header);
        printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);

    }
    file_close(file);
    printf("Exiting receive command\r\n");
    return;

}