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
#include "encryption.h"

#define DEBUG_PRINT_DISPATCHER (0)
#define DEBUG_PRINT_WORKER     (0)

extern char *dfs_name;

/*******************************************************************************
 * DFS state machines
 ******************************************************************************/
void sm_server(int sockfd_listen, conf_results_dfs_t conf)
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
                if(threading_create_dispatcher(new_fd, conf) != 0)
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
    conf_results_dfs_t conf = args.conf;

    // variable to enable timeout
    bool is_timeout = false;

    // variables to store socket read results
    char in_buf[PACKET_SIZE];
    int recvbytes;

    DIR *dir = NULL;

    char path_buffer[60];
    char results_buffer[60];
    char output_buffer[PACKET_SIZE];

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
            return 0;
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

        // 4. Verify if the command has a valid user+pass combo
        int is_match = 0;
        printf("user_name: %s; length: %ld\r\n", pkt.user_name, strlen(pkt.user_name));
        printf("password: %s; length: %ld\r\n", pkt.password, strlen(pkt.password));
        printf("user1: %s; length: %ld\r\n", conf.user1, strlen(conf.user1));
        printf("conf password: %s; length: %ld\r\n", conf.pass1, strlen(conf.pass1));
        if(strcmp(pkt.user_name, conf.user1) == 0)
        {
            if(strcmp(pkt.password, conf.pass1) == 0)
            {
                is_match = 1;
            }
        }
        else if(strcmp(pkt.user_name, conf.user2) == 0)
        {
            if(strcmp(pkt.password, conf.pass2) == 0)
            {
                is_match = 1;
            }
        }
        else if(strcmp(pkt.user_name, conf.user3) == 0)
        {
            if(strcmp(pkt.password, conf.pass3) == 0)
            {
                is_match = 1;
            }
        }
        else if(strcmp(pkt.user_name, conf.user4) == 0)
        {
            if(strcmp(pkt.password, conf.pass4) == 0)
            {
                is_match = 1;
            }
        }
        else if(strcmp(pkt.user_name, conf.user5) == 0)
        {
            if(strcmp(pkt.password, conf.pass5) == 0)
            {
                is_match = 1;
            }
        }
    
        
        // 5. If the request is from a known user, ensure there is a directory 
        // for them, if not, create the directory.
        if(is_match)
        {
            printf("Valid user found\r\n");
            char directory_string[30];
            sprintf(directory_string, "./%s/%s", dfs_name, pkt.user_name);
            dir = file_open_dir(directory_string);
            if(dir == NULL)
            {
                printf("Failed to open %s\r\n", directory_string);
                sock_close(args.fd_client);
                return 0;
            }
        }
        else
        {
            printf("Invalid user detected. Closing connection to client\r\n");
            sock_close(args.fd_client);
            return 0;
        }

        // (1) Process request
        switch(pkt.cmd_header)
        {
            case CMD_GET_PKT:
                printf("%s#%d: Made it to GET cmd\r\n", dfs_name, args.thread_id);
                // TODO: ARMANDO

                // 1. Initialize variables
                conf_results_t conf;
                char file_name[50];
                char file_path[70];
                strcpy(conf.user_name, pkt.user_name);
                strcpy(conf.password, pkt.password);

                // 2. Check if a file chunk 1 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 70);
                sprintf(file_name, ".%s.1", pkt.file_name);
                sprintf(file_path, "./%s/%s/%s", dfs_name, pkt.user_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 3. Check if a file chunk 2 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 70);
                sprintf(file_name, ".%s.2", pkt.file_name);
                sprintf(file_path, "./%s/%s/%s", dfs_name, pkt.user_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 4. Check if a file chunk 3 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 70);
                sprintf(file_name, ".%s.3", pkt.file_name);
                sprintf(file_path, "./%s/%s/%s", dfs_name, pkt.user_name, file_name);
                if(file_exists(file_path))
                {
                    printf("found %s\r\n", file_path);
                    sm_send(file_path, conf, args.fd_client, 1);
                }

                // 5. Check if a file chunk 4 exists
                memset(file_name, 0, 50);
                memset(file_path, 0, 70);
                sprintf(file_name, ".%s.4", pkt.file_name);
                sprintf(file_path, "./%s/%s/%s", dfs_name, pkt.user_name, file_name);
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

                // Get ls
                sprintf(path_buffer, "./%s/%s", dfs_name, pkt.user_name);
                recvbytes = print_directory(path_buffer, results_buffer);
                printf("Results:\r\n%s\r\nbytes = %d\r\n", results_buffer, recvbytes);

                // Send results
                pkt.payload_header = recvbytes;
                memset(pkt.payload, 0, PAYLOAD_CHUNK_SIZE);
                memcpy(pkt.payload, results_buffer, recvbytes);

                printf("===payload: %s\r\n", pkt.payload);

                recvbytes = packet_convert_to_buffer(pkt, output_buffer);
                printf("message size = %d\r\n", recvbytes);

                recvbytes = sock_send(args.fd_client, output_buffer, recvbytes);
                printf("bytes sent = %d\r\n", recvbytes);
            
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

    strcpy(pkt.user_name, conf.user_name);
    strcpy(pkt.password, conf.password);
    pkt.cmd_header = CMD_GET_PKT;
    pkt.crc32_header = 9999;
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
    // Send the packet to DFS2
    bytes_sent = sock_send(fd.dfs2, out_buffer, packet_size);

    printf("packet_size: %d, bytes_sent: %d\r\n", packet_size, bytes_sent);

    // Wait for responses from DFS2
    if(sm_receive_pieces(fd.dfs2) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }
    if(sm_receive_pieces(fd.dfs2) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }

    /***************************************************************************
     * Get fle pieces from DFS3
     **************************************************************************/
    // Send the packet to DFS3
    bytes_sent = sock_send(fd.dfs3, out_buffer, packet_size);

    printf("packet_size: %d, bytes_sent: %d\r\n", packet_size, bytes_sent);

    // Wait for responses from DFS3
    if(sm_receive_pieces(fd.dfs3) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }
    if(sm_receive_pieces(fd.dfs3) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }

    /***************************************************************************
     * Get file pieces from DFS4
     **************************************************************************/
    // Send the packet to DFS4
    bytes_sent = sock_send(fd.dfs4, out_buffer, packet_size);

    printf("packet_size: %d, bytes_sent: %d\r\n", packet_size, bytes_sent);

    // Wait for responses from DFS4
    if(sm_receive_pieces(fd.dfs4) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }
    if(sm_receive_pieces(fd.dfs4) == -1)
    {
        printf("Client: Failed to get %s\r\n", file_name);
    }
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

        if(is_server)
        {
            // 5.5 Encrypt payload
            encrypt_decrypt(pkt.payload, pkt.payload_header, pkt.password, strlen(pkt.password));
        }

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

        if(is_server)
        {
            // 5.5 Encrypt payload
            encrypt_decrypt(pkt.payload, pkt.payload_header, pkt.password, strlen(pkt.password));
        }

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

                if(is_server)
                {
                    // 11.5 Encrypt payload
                    encrypt_decrypt(file_buffer, (pkt.payload_header - file_bytes_sent), pkt.password, strlen(pkt.password));
                }

                // 12. Send the pure payload
                bytes_sent = sock_send(fd, file_buffer, (pkt.payload_header - file_bytes_sent));
                
                file_bytes_sent += (pkt.payload_header - file_bytes_sent);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
            else
            {
                file_read(file_buffer, file, PAYLOAD_CHUNK_SIZE);
                file_bytes_sent += PAYLOAD_CHUNK_SIZE;

                if(is_server)
                {
                    // 11.5 Encrypt payload
                    encrypt_decrypt(file_buffer, PAYLOAD_CHUNK_SIZE, pkt.password, strlen(pkt.password));
                }

                bytes_sent = sock_send(fd, file_buffer, PAYLOAD_CHUNK_SIZE);
                printf("Sent: %d; File transfer: %d/%d sent\r\n", bytes_sent, file_bytes_sent, pkt.payload_header);
            }
        }
    }

    return;
}

int sm_merge(char *file_name, conf_results_t conf)
{
    FILE *new_file;
    FILE *chunk_file;
    char file_chunk_name[50];
    int file_size;
    int data_chunk_size = 512;
    char data_chunk[data_chunk_size];
    int bytes;

    // 0. Design
    printf("Client: Attempting to merge file.\r\n");

    // 1. Check if the file already exists locally
    if(file_exists(file_name) == 1)
    {
        printf("Client: %s already exists.\r\n", file_name);
        return -1;
    }

    // 2. Check if we have enough chunks to combine.
    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.1", file_name);
    if(file_exists(file_chunk_name) == 0)
    {
        printf("Client: missing file %s\r\n", file_chunk_name);
        return -1;
    }

    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.2", file_name);
    if(file_exists(file_chunk_name) == 0)
    {
        printf("Client: missing file %s\r\n", file_chunk_name);
        return -1;
    }

    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.3", file_name);
    if(file_exists(file_chunk_name) == 0)
    {
        printf("Client: missing file %s\r\n", file_chunk_name);
        return -1;
    }

    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.4", file_name);
    if(file_exists(file_chunk_name) == 0)
    {
        printf("Client: missing file %s\r\n", file_chunk_name);
        return -1;
    }

    // 3. Open the new file.
    new_file = file_open_create(file_name);
    if(new_file == NULL)
    {
        printf("Client: Failed to create %s\r\n", file_name);
        return -1;
    }

    // 4. Open CHUNK1 and write to new file;
    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.1", file_name);
    chunk_file = file_open(file_chunk_name, 0);
    if(chunk_file == NULL)
    {
        printf("Client: Failed to open %s\r\n", file_chunk_name);
        return -1;
    }

    file_size = file_get_size(chunk_file);
    printf("Client: File %s is %d bytes in size\r\n", file_chunk_name, file_size);

    if(file_size < data_chunk_size)
    {
        memset(data_chunk, 0, data_chunk_size);
        bytes = file_read(data_chunk, chunk_file, file_size);
        bytes = file_write(data_chunk, new_file, bytes);
        printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
    }
    else
    {
        while(file_size > 0)
        {
            memset(data_chunk, 0, data_chunk_size);
            if(file_size > data_chunk_size)
            {
                bytes = file_read(data_chunk, chunk_file, data_chunk_size);
            }
            else
            {
                bytes = file_read(data_chunk, chunk_file, file_size);
            }
            
            bytes = file_write(data_chunk, new_file, bytes);
            file_size -= bytes;
            printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
            printf("Clinet: Bytes remaining = %d\r\n", file_size);
        }

    }

    file_close(chunk_file);

    // 5. Open CHUNK2 and write to new file;
    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.2", file_name);
    chunk_file = file_open(file_chunk_name, 0);
    if(chunk_file == NULL)
    {
        printf("Client: Failed to open %s\r\n", file_chunk_name);
        return -1;
    }

    file_size = file_get_size(chunk_file);
    printf("Client: File %s is %d bytes in size\r\n", file_chunk_name, file_size);

    if(file_size < data_chunk_size)
    {
        memset(data_chunk, 0, data_chunk_size);
        bytes = file_read(data_chunk, chunk_file, file_size);
        bytes = file_write(data_chunk, new_file, bytes);
        printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
    }
    else
    {
        while(file_size > 0)
        {
            memset(data_chunk, 0, data_chunk_size);
            if(file_size > data_chunk_size)
            {
                bytes = file_read(data_chunk, chunk_file, data_chunk_size);
            }
            else
            {
                bytes = file_read(data_chunk, chunk_file, file_size);
            }
            
            bytes = file_write(data_chunk, new_file, bytes);
            file_size -= bytes;
            printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
            printf("Clinet: Bytes remaining = %d\r\n", file_size);
        }

    }

    file_close(chunk_file);

    // 6. Open CHUNK3 and write to new file;
    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.3", file_name);
    chunk_file = file_open(file_chunk_name, 0);
    if(chunk_file == NULL)
    {
        printf("Client: Failed to open %s\r\n", file_chunk_name);
        return -1;
    }

    file_size = file_get_size(chunk_file);
    printf("Client: File %s is %d bytes in size\r\n", file_chunk_name, file_size);

    if(file_size < data_chunk_size)
    {
        memset(data_chunk, 0, data_chunk_size);
        bytes = file_read(data_chunk, chunk_file, file_size);
        bytes = file_write(data_chunk, new_file, bytes);
        printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
    }
    else
    {
        while(file_size > 0)
        {
            memset(data_chunk, 0, data_chunk_size);
            if(file_size > data_chunk_size)
            {
                bytes = file_read(data_chunk, chunk_file, data_chunk_size);
            }
            else
            {
                bytes = file_read(data_chunk, chunk_file, file_size);
            }
            
            bytes = file_write(data_chunk, new_file, bytes);
            file_size -= bytes;
            printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
            printf("Clinet: Bytes remaining = %d\r\n", file_size);
        }

    }

    file_close(chunk_file);

    // 7. Open CHUNK4 and write to new file;
    memset(file_chunk_name, 0, 50);
    sprintf(file_chunk_name, ".%s.4", file_name);
    chunk_file = file_open(file_chunk_name, 0);
    if(chunk_file == NULL)
    {
        printf("Client: Failed to open %s\r\n", file_chunk_name);
        return -1;
    }

    file_size = file_get_size(chunk_file);
    printf("Client: File %s is %d bytes in size\r\n", file_chunk_name, file_size);

    if(file_size < data_chunk_size)
    {
        memset(data_chunk, 0, data_chunk_size);
        bytes = file_read(data_chunk, chunk_file, file_size);
        bytes = file_write(data_chunk, new_file, bytes);
        printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
    }
    else
    {
        while(file_size > 0)
        {
            memset(data_chunk, 0, data_chunk_size);
            if(file_size > data_chunk_size)
            {
                bytes = file_read(data_chunk, chunk_file, data_chunk_size);
            }
            else
            {
                bytes = file_read(data_chunk, chunk_file, file_size);
            }
            
            bytes = file_write(data_chunk, new_file, bytes);
            file_size -= bytes;
            printf("Client: Wrote %d bytes of %s to %s\r\n", bytes, file_chunk_name, file_name);
            printf("Clinet: Bytes remaining = %d\r\n", file_size);
        }

    }

    file_close(chunk_file);

    // 8. Close the new file
    file_close(new_file);

    // 9. Delete the file chunks

    return 0;

}
/*******************************************************************************
 * 
 ******************************************************************************/
// This function will receive a file from socket connection `fd`. The initial 
// parameters of the file are provided to the function in `pkt`.
void sm_receive(int fd, Packet pkt, int is_server)
{
    FILE *file = NULL;
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
        sprintf(file_path, "./%s/%s/%s", dfs_name, pkt.user_name, pkt.file_name);
    }
    else
    {
        get_filename(pkt.file_name);
        strcpy(file_path, pkt.file_name);
    }
    

    // Open/create file
    int already_exists = file_exists(file_path);
    printf("Does the file %s exist already? = %d\r\n", file_path, already_exists);
    if(!already_exists)
    {
        file = file_open_create(file_path);
        if(file == NULL)
        {
            printf("Failed to create file %s\r\n", file_path);
            return;
        }
    }

    // If the payload is going to require more than 1 transfer
    if(pkt.payload_header > PAYLOAD_CHUNK_SIZE)
    {
        file_bytes_recv = PAYLOAD_CHUNK_SIZE;

        // Write initial payload
        if(!already_exists)
        {
            if(is_server)
            {
                // Decrypt payload
                encrypt_decrypt(pkt.payload, file_bytes_recv, pkt.password, strlen(pkt.password));
            }
            // Save payload
            bytes_saved = file_write(pkt.payload, file, PAYLOAD_CHUNK_SIZE);
            printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);
        }

        while(file_bytes_recv < pkt.payload_header)
        {
            memset(in_buf, 0, PAYLOAD_CHUNK_SIZE);
            if((pkt.payload_header-file_bytes_recv) > PAYLOAD_CHUNK_SIZE)
            {
                bytes_recv = sock_read(fd, in_buf, PAYLOAD_CHUNK_SIZE, 0);
            }
            else
            {
                bytes_recv = sock_read(fd, in_buf, (pkt.payload_header-file_bytes_recv) , 0);
            }
            
            if(!already_exists)
            {
                if(is_server)
                {
                    // Decrypt payload
                    encrypt_decrypt(in_buf, (pkt.payload_header-file_bytes_recv), pkt.password, strlen(pkt.password));
                }
                // Save payload
                bytes_saved += file_write(in_buf, file, bytes_recv);
                printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);
            }
            file_bytes_recv += bytes_recv;
        }
    }
    // payload requires only 1 transfer
    else
    {
        // Write initial payload
        if(!already_exists)
        {
            if(is_server)
            {
                // Decrypt payload
                encrypt_decrypt(pkt.payload, pkt.payload_header, pkt.password, strlen(pkt.password));
            }
            // Save payload
            bytes_saved = file_write((char *)pkt.payload, file, pkt.payload_header);
            printf("Saved %d/%d bytes to %s\r\n", bytes_saved, pkt.payload_header, file_path);
        }

    }

    if(!already_exists)
    {
        file_close(file);
    }

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
    printf("Client: waiting for message from server\r\n");
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

int sm_client_ls(fd_dfs_t fd, conf_results_t conf, char *path)
{
    // 1. Create command.
    // 2. Send command to all servers.
    // 3. Wait for response from each server.
    // 4. Process results.
    // 5. Display result.

    char in_buffer[PACKET_SIZE];

    uint32_t bytes_sent = 0;
    uint32_t bytes_recv = 0;
    uint32_t packet_size = 0;
    char out_buffer[PACKET_SIZE];
    char *token;

    // Generate the CMD header
    Packet pkt = packet_get_default();
    Packet pkt1 = packet_get_default();
    Packet pkt2 = packet_get_default();
    Packet pkt3 = packet_get_default();
    Packet pkt4 = packet_get_default();

    strcpy(pkt.user_name, conf.user_name);
    strcpy(pkt.password, conf.password);
    pkt.cmd_header = CMD_LS_PKT;
    pkt.crc32_header = 9999;
    strcpy(pkt.file_name, path);

    /***************************************************************************
     * Send CMD to DFS1, wait for DFS1 response
     **************************************************************************/
    memset(out_buffer, 0, PACKET_SIZE);
    memset(in_buffer, 0, PACKET_SIZE);
    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet to DFS1
    bytes_sent = sock_send(fd.dfs1, out_buffer, packet_size);

    // Wait for response from DFS1
    bytes_recv = sock_read(fd.dfs1, in_buffer, PACKET_SIZE, 0);
    if(bytes_recv == -1)
    {
        printf("Client: Error receiving from DFS1\r\n");
        return -1;
    }

    // Parse input
    pkt1 = packet_parse_packet(in_buffer, PACKET_SIZE);
    if(packet_is_default(pkt1) == true)
    {
        printf("Client: Failed to parse response from DFS1\r\n");
    }
    else
    {
        // Print result
        printf("DFS1 Contents:\r\n");
        token = strtok(pkt1.payload, " ");
        do
        {
            printf("%s\r\n", token);
        } while ((token = strtok(NULL, " ")) != NULL);
        printf("\r\n");
    }
    
    /***************************************************************************
     * Send CMD to DFS2, wait for DFS2 response
     **************************************************************************/
    memset(out_buffer, 0, PACKET_SIZE);
    memset(in_buffer, 0, PACKET_SIZE);
    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet to DFS2
    bytes_sent = sock_send(fd.dfs2, out_buffer, packet_size);

    // Wait for response from DFS2
    bytes_recv = sock_read(fd.dfs2, in_buffer, PACKET_SIZE, 0);
    if(bytes_recv == -1)
    {
        printf("Client: Error receiving from DFS2\r\n");
        return -1;
    }

    // Parse input
    pkt2 = packet_parse_packet(in_buffer, PACKET_SIZE);
    if(packet_is_default(pkt2) == true)
    {
        printf("Client: Failed to parse response from DFS2\r\n");
    }
    else
    {
        // Print result
        printf("DFS2 Contents:\r\n");
        token = strtok(pkt2.payload, " ");
        do
        {
            printf("%s\r\n", token);
        } while ((token = strtok(NULL, " ")) != NULL);
        printf("\r\n");
    }

    /***************************************************************************
     * Send CMD to DFS3, wait for DFS3 response
     **************************************************************************/
    memset(out_buffer, 0, PACKET_SIZE);
    memset(in_buffer, 0, PACKET_SIZE);
    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet to DFS3
    bytes_sent = sock_send(fd.dfs3, out_buffer, packet_size);

    // Wait for response from DFS3
    bytes_recv = sock_read(fd.dfs3, in_buffer, PACKET_SIZE, 0);
    if(bytes_recv == -1)
    {
        printf("Client: Error receiving from DFS3\r\n");
        return -1;
    }

    // Parse input
    pkt3 = packet_parse_packet(in_buffer, PACKET_SIZE);
    if(packet_is_default(pkt3) == true)
    {
        printf("Client: Failed to parse response from DFS3\r\n");
    }
    else
    {
        // Print result
        printf("DFS3 Contents:\r\n");
        token = strtok(pkt3.payload, " ");
        do
        {
            printf("%s\r\n", token);
        } while ((token = strtok(NULL, " ")) != NULL);
        printf("\r\n");
    }

    /***************************************************************************
     * Send CMD to DFS4, wait for DFS4 response
     **************************************************************************/
    memset(out_buffer, 0, PACKET_SIZE);
    memset(in_buffer, 0, PACKET_SIZE);
    // Generate packet to send
    packet_size = packet_convert_to_buffer(pkt, out_buffer);

    // Send the packet to DFS4
    bytes_sent = sock_send(fd.dfs4, out_buffer, packet_size);

    // Wait for response from DFS4
    bytes_recv = sock_read(fd.dfs4, in_buffer, PACKET_SIZE, 0);
    if(bytes_recv == -1)
    {
        printf("Client: Error receiving from DFS4\r\n");
        return -1;
    }

    // Parse input
    pkt4 = packet_parse_packet(in_buffer, PACKET_SIZE);
    if(packet_is_default(pkt4) == true)
    {
        printf("Client: Failed to parse response from DFS4\r\n");
    }
    else
    {
        // Print result
        printf("DFS4 Contents:\r\n");
        token = strtok(pkt4.payload, " ");
        do
        {
            printf("%s\r\n", token);
        } while ((token = strtok(NULL, " ")) != NULL);
        printf("\r\n");
    }

    return 0;
}