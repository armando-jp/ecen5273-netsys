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
#include "utils.h"

#define DEBUG_PRINT_DISPATCHER (0)
#define DEBUG_PRINT_WORKER     (0)

extern char *dfs_name;

/*******************************************************************************
 * DFS state machines
 ******************************************************************************/
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

void *sm_server_thread(void *p_args)
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

                // 1. Initialize variables
                conf_results_t conf;
                char file_name[50];
                char file_path[60];
                strcpy(conf.user_name, pkt.user_name);
                strcpy(conf.password, pkt.password);

                // 2. Check if a file chunk 1 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 60);
                sprintf(file_name, ".%s.1", pkt.file_name);
                sprintf(file_path, "./%s/%s", dfs_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 3. Check if a file chunk 2 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 60);
                sprintf(file_name, ".%s.2", pkt.file_name);
                sprintf(file_path, "./%s/%s", dfs_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 4. Check if a file chunk 3 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 60);
                sprintf(file_name, ".%s.3", pkt.file_name);
                sprintf(file_path, "./%s/%s", dfs_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 5. Check if a file chunk 4 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 60);
                sprintf(file_name, ".%s.4", pkt.file_name);
                sprintf(file_path, "./%s/%s", dfs_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

            break;

            case CMD_PUT_PKT:
                printf("%s#%d: Made it to PUT cmd\r\n", dfs_name, args.thread_id);
                sm_receive(args.fd_client, pkt, 1);

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

    /***************************************************************************
     * Get file pieces from DFS1
     **************************************************************************/
    // Generate the CMD header
    Packet pkt = packet_get_default();

    strcpy(pkt.user_name, "Amy");
    strcpy(pkt.password, "KoolPass");
    pkt.cmd_header = CMD_GET_PKT;
    pkt.crc32_header = 1198457;
    strcpy(pkt.file_name, file_name);

    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet to DFS1
    bytes_sent = sock_send(fd.dfs1, out_buffer, packet_size);

    printf("packet_size: %d, bytes_sent: %d\r\n", packet_size, bytes_sent);

    // Wait for responses from DFS1
    if(sm_receive_pieces(fd.dfs1) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }
    if(sm_receive_pieces(fd.dfs1) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }

    /***************************************************************************
     * Get fle pieces from DFS2
     **************************************************************************/

    /***************************************************************************
     * Get fle pieces from DFS3
     **************************************************************************/

    /***************************************************************************
     * Get fle pieces from DFS4
     **************************************************************************/
    return;
}

// This function will send the `file_name` to socket connection `fd` with 
// user_name and passowrd from `conf`.
void sm_send(char *file_name, conf_results_t conf, int fd, int is_server)
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

    if(is_server)
    {
        get_filename(file_name);
        printf("extracted file name: %s\r\n", file_name);
        strcpy(pkt.file_name, file_name);
    }
    else
    {
        strcpy(pkt.file_name, file_name);
    }
    
    pkt.payload_header = file_get_size(file);

    // 3: Print pkt
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
        bytes_sent = sock_send(fd, out_buffer, out_buffer_size);
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
        bytes_sent = sock_send(fd, out_buffer, out_buffer_size);

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
                bytes_sent = sock_send(fd, file_buffer, (pkt.payload_header - file_bytes_sent));
                
                file_bytes_sent += (pkt.payload_header - file_bytes_sent);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
            else
            {
                file_read(file_buffer, file, PAYLOAD_CHUNK_SIZE);
                file_bytes_sent += PAYLOAD_CHUNK_SIZE;

                bytes_sent = sock_send(fd, file_buffer, PAYLOAD_CHUNK_SIZE);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
        }
    }

    return;
}

/*******************************************************************************
 * 
 ******************************************************************************/
// This function will receive a file from socket connection `fd`. The initial 
// parameters of the file are provided to the function in `pkt`.
void sm_receive(int fd, Packet pkt, int is_server)
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

    // Process filename 
    char file_path[FILE_NAME_SIZE + 21];
    if(is_server)
    {
        sprintf(file_path, "./%s/%s", dfs_name, pkt.file_name);
    }
    else
    {
        get_filename(pkt.file_name);
        strcpy(file_path, pkt.file_name);
    }
    

    // Open/create file
    file = file_open_create(file_path);
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

int sm_receive_pieces(int fd)
{
    // FILE *file;
    Packet pkt;
    // uint32_t bytes_saved = 0;
    uint32_t bytes_recv = 0;
    char in_buf[PACKET_SIZE];
    // uint32_t file_bytes_recv = 0;
    int is_timeout = 0;

    // 1. Get an incomming message.
    memset(in_buf, 0, PACKET_SIZE);
    printf("Client: waiting for message from server");
    bytes_recv = sock_read(fd, in_buf, PACKET_SIZE, is_timeout);
    if(bytes_recv == -1)
    {
        if(is_timeout)
        {
            printf("Client: TIMEOUT! Closing connection to server.\n");
            return -1;
            // current_event = evt_timeout;
        }
        else
        {
            printf("Client: Unexpected error during recv.\n");
            return -1;
        }
    }
    else if(bytes_recv == 0)
    {
        // current_event = evt_close_request;
        printf("Client: Got EOF from client. Shutting down.\n");
        return -1;
    }
    else
    {
        // current_event = evt_message_received;
        printf("Client: GOT %d BYTES FROM CLIENT (DFC COMMAND)\n", bytes_recv);
    }

    // 2. Parse received command
    printf("Client: Attempting to parse DFC command.\n");
    pkt = packet_parse_packet(in_buf, bytes_recv);
    if(packet_is_default(pkt))
    {
        // invalid request received
        printf("Client: received invalid request\r\n");
        return -1;
    }  

    // 3. process command
    packet_print(pkt);

    // 4. Receive the file chunk
    sm_receive(fd, pkt, 0);

    return 0;
}