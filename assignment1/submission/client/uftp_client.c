#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include "../include/timer.h"
#include "../include/file.h"
#include "../include/crc.h"
#include "../include/packet.h"
#include "../include/state_machine.h"

#define DEBUG         (0)


int main(int argc, char *argv[]) 
{
    // variables for user input processing
    int ret;
    uint32_t crc32_calc;

    char *user_resp = NULL;
    char user_param[MAX_USER_ARG];


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
    ret = sock_init_udp_struct(port_str, ip_str, true);
    if(!ret)
    {
        printf("Error in sock_init_udp_struct\n");
    }
    if(!sock_create_socket())
    {
        printf("Error in sock_create_scket()\n");
    }
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
                sm_client_get();
                break;

            case CMD_PUT:
                sm_client_put();
                break;

            case CMD_DELETE:
                printf("In delete\n");
                /***************************************************************
                 * Send CMD and get server ACK
                 **************************************************************/
                // 1. generate filtered version of user commands
                cli_generate_filtered_usr_cmd("delete", user_param);

                // 2. fill packet struct fields
                packet_write_payload_size(cli_get_filtered_usr_cmd_size());
                packet_write_payload(
                    cli_get_filtered_usr_cmd(), 
                    packet_get_payload_size()
                );
                packet_write_crc32(
                    crc_generate(
                        (char *)packet_get_struct(), 
                        packet_get_packet_size_for_crc()
                    )
                );

                // 3. generate packet buffer
                ret = packet_generate();

                ret = 0;
                sock_clear_input_buffer();
                while(ret <= 0)
                {
                    // 4. send cmd packet to server
                    ret = sock_sendto(packet_get_buf(), packet_get_total_size(), true);
                    printf("Sent CMD\n");

                    // 5. wait for ack from server
                    printf("Waiting for ACK from server\n");
                    socket_init_timeout();
                    sock_enable_timeout();
                    ret = sock_recv(1);
                    printf("RET = %d\n", ret);
                }

                // 6. parse server response
                packet_parse(sock_get_in_buf());

                // 7. verify that payload contents are correct (crc32 check)
                crc32_calc = crc_generate(
                    packet_get_payload(), 
                    packet_get_payload_size()
                );
                if(crc32_calc != packet_get_crc32())
                {
                    printf("CRC32 mismatch, corrupted packet!");
                    continue;
                }

                // 8. check that the payload is ACK
                if ( strcmp(packet_get_payload(), "ACK") != 0 )
                {
                    // ack was not received. start over.
                    printf("ACK NOT RECEIVED\n");
                    continue;
                }
                break;

            case CMD_LS:
                sm_client_ls();
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