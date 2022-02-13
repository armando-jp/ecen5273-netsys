#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include "../include/timer.h"
#include "../include/file.h"
#include "../include/crc.h"
#include "../include/packet.h"
#include "../include/state_machine.h"

#include <stdio.h>
#include <errno.h>

#define DEBUG (0)

int ret;
char cmd_params[MAX_USER_ARG];
extern char in_buf[MAX_IN_BUF_LEN];
uint32_t crc32_calc;

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

    // bind to port
    sock_init_udp_struct(port_str, NULL, false);
    sock_create_socket();
    sock_bind();
    sock_free_udp_struct();

    // configure timeout for RECV
    // socket_init_timeout();

    // enter super loop
    while(1)
    {
        printf("listener: waiting to recvfrom...\n");
        // block until message is received
        sock_clear_input_buffer();
        ret = sock_recv(0);

        // parse input sock buffer into packet struct
        packet_parse(sock_get_in_buf());
        // packet_print_struct();


        // verify that payload contents are correct
        crc32_calc = crc_generate(packet_get_payload(), packet_get_payload_size());
        if(crc32_calc != packet_get_crc32())
        {
            printf("CRC32 mismatch, corrupted packet!");
            continue;
        }
        printf("CRC32 matched!\n");

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
                sm_server_put();
                break;

            case CMD_DELETE:
                 // 1. generate ACK packet.
                printf("Generating ACK packet\n");
                packet_write_payload_size(sizeof("ACK"));
                packet_write_payload("ACK", packet_get_payload_size());
                packet_write_crc32(
                    crc_generate(
                        packet_get_payload(), 
                        packet_get_payload_size()
                    )
                );
                packet_generate();

                // 2. send ACK packet
                ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                // Start DELETE operation
                printf("Performing DELETE command with param \"%s\"\n", cmd_params);

                // preperations
                // 1. open file with 
                if(file_delete(cmd_params) == -1)
                {
                    printf("Failed to delete file %s\n", cmd_params);
                }
                else
                {
                    printf("%s deleted sucessfully\n", cmd_params);
                }
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