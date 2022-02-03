#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include "../include/timer.h"
#include "../include/file.h"
#include "../include/crc.h"
#include "../include/packet.h"

#include <stdio.h>

int ret;
char cmd_params[MAX_USER_ARG];
extern char in_buf[MAX_IN_BUF_LEN];
uint32_t crc32;

int main(int argc, char *argv[]) 
{
    // verify the correct number of arguments
    if(argc != 2)
    {
        msg_bad_args_server();
        return 0;
    }

    // save arguments to named variables
    char *port_str = argv[1];

    // verify port string
    if(!is_port(port_str))
    {
        msg_invalid_port(port_str);
        msg_bad_args_server();
        return 0;
    }

    sock_init_udp_struct(port_str, NULL, false);
    sock_create_socket();
    sock_bind();
    sock_free_udp_struct();

    // enter super loop
    while(1)
    {
        printf("listener: waiting to recvfrom...\n");
        // block until message is received
        sock_clear_input_buffer();
        ret = sock_recv();

        // parse input sock buffer into packet struct
        packet_parse(sock_get_in_buf());
        packet_print_struct();

        // verify that payload contents are correct
        crc32 = crc_generate(packet_get_payload(), packet_get_payload_size());
        if(crc32 != packet_get_crc32())
        {
            printf("CRC32 mismatch, corrupted packet!");
            continue;
        }

    


        // process command
        ret = get_command(packet_get_payload(), cmd_params);
        if(!ret)
        {
            msg_bad_command();
            continue;
        }

        switch(ret)
        {
            case CMD_GET:
                // perform get operation
                printf("Performing GET command with param \"%s\"\n", cmd_params);
                break;

            case CMD_PUT:
                // the command we received was correct
                // 1. generate ACK packet.
                // 2. send ACK packet


                // perform put operation
                printf("Performing PUT command with param \"%s\"\n", cmd_params);

                // // receive a chunk
                // sock_clear_input_buffer();
                // ret = sock_recv();

                // // parse packet
                // packet_parse(sock_get_in_buf());

                // // print parsed packet
                // packet_print_struct();

                // // get crc32 for received chunks
                // uint32_t crc32 = crc_generate(packet_get_payload(), packet_get_payload_size());
                // printf("Calculated CRC32: %u\n", crc32);

                // // print OG payload
                // printf("Unparsed payload contents\n");
                // packet_print(sock_get_in_buf(), ret);
                
                break;

            case CMD_DELETE:
                // perform delete operation
                printf("Performing DELETE command with param \"%s\"\n", cmd_params);
                break;

            case CMD_LS:
                // perform ls operation
                printf("Performing LS command\n");
                break;

            // case CMD_EXIT:
            //     msg_app_closing();
            //     return 0;
        }
    }



    sock_close_socket();

    return 0;
}