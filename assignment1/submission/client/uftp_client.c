#include <stdio.h>
#include <unistd.h>

#define CHUNK_SIZE (500)
#define MAX_PAYLOAD (4 + CHUNK_SIZE + 4)

#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include "../include/timer.h"
#include "../include/file.h"
#include "../include/crc.h"
#include "../include/packet.h"


char packet[MAX_PAYLOAD];
extern struct t_eventData eventData;

int main(int argc, char *argv[]) 
{
    // variables for user input processing
    uint32_t ret;
    int res;
    char *user_resp = NULL;
    char user_param[MAX_USER_ARG];
    char payload[MAX_PAYLOAD];
    char *file_buf = NULL;

    // verify the correct number of arguments
    if(argc != 3)
    {
        msg_bad_args_client();
        return 0;
    }

    // save arguments to named variables
    char *ip_str = argv[1];
    char *port_str = argv[2];

    // verify IP string
    // TODO: This only applies if IPv4 is supplied and won't work
    // with a string (i.e. www.google.com) consider removing and checking 
    // for valid address later in program?
    if(!is_ip(ip_str))
    {
        msg_invalid_ip(ip_str);
        msg_bad_args_client();
        return 0;
    }
    // verify port string
    if(!is_port(port_str))
    {
        msg_invalid_port(port_str);
        msg_bad_args_client();
        return 0;
    }

    // generate struct needed for connection
    // TODO: ERROR HANDLING
    ret = sock_init_udp_struct(port_str, ip_str, true);
    sock_create_socket();
    // sock_print_ip();
    // ret = sock_sendto("Hello from Client!", 19);
    // printf("Sent %d bytes to %s\n", ret, ip_str);
    sock_free_udp_struct();
    
    // enter super loop
    while(1)
    {
        // display main menu and process user response
        cli_display_main_menu();
        user_resp = cli_get_user_response();
        ret = get_command(user_resp, user_param);
        
        if(!ret)
        {
            msg_bad_command();
            continue;
        }

        switch(ret)
        {
            case CMD_GET:
                // perform get operation
                // calculate payload size

                // snprintf(payload, MAX_PAYLOAD, "get %s",user_param);
                // ret = sock_sendto(payload, strlen(payload));
                // printf("Sent %d bytes to %s\n", ret, ip_str);
                break;

            case CMD_PUT:
                // send the command to prep the server
                ret = packet_command(packet, "put", user_param);
                printf("packet_size = %d\n", ret);
                packet_print(packet, ret);

                ret = sock_sendto(packet, ret);
                printf("Sent %u bytes to %s\n", ret, ip_str);
                printf("\n");


                // display file information
                file_open(user_param);
                printf("%s size (bytes): %d\n", user_param, file_get_size());

                // file_print_all(CHUNK_SIZE);

                // generate and print crc32
                uint32_t crc32 = crc_generate_file(file_get_fp());
                printf("File CRC: %u\n", crc32);

                // reset fp for actual read
                file_reset_fileptr();
                ret = file_read_chunk(CHUNK_SIZE);
                printf("Chunk read: %d\n", ret);
                crc32 = crc_generate(file_get_file_buf(), ret);
                printf("Chunk CRC32: %u\n", crc32);
                uint32_t sent = packet_generate(
                    packet, 
                    CHUNK_SIZE,
                    file_get_file_buf(),
                    ret, 
                    crc32
                );
                printf("Packet size: %d\n", sent);
                packet_print(packet, sent);

                // send file to server
                sleep(1);
                res = sock_sendto(packet, sent);
                printf("Sent %u bytes to %s\n", res, ip_str);
                break;

            case CMD_DELETE:
                // perform delete operation
                snprintf(payload, MAX_PAYLOAD, "delete %s",user_param);
                ret = sock_sendto(payload, strlen(payload));
                printf("Sent %d bytes to %s\n", ret, ip_str);
                break;

            case CMD_LS:
                // perform ls operation
                snprintf(payload, MAX_PAYLOAD, "ls %s",user_param);
                ret = sock_sendto(payload, strlen(payload));
                printf("Sent %d bytes to %s\n", ret, ip_str);
                break;

            case CMD_EXIT:
                msg_app_closing();
                return 0;
        }

        // printf("CMD: %d\nBUF: %s\nPARAM: %s\n", ret, user_resp, user_param);

    }
    


    //test_greeting(0);
    return 0;
}