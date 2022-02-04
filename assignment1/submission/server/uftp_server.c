#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include "../include/timer.h"
#include "../include/file.h"
#include "../include/crc.h"
#include "../include/packet.h"

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
    socket_init_timeout();

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
                sock_enable_timeout();
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

                // Start PUT operation
                printf("Performing PUT command with param \"%s\"\n", cmd_params);

                // preperations
                // 1. open file with 
                file_open(cmd_params, 1);
                
                while(1)
                {
                    // 1. receive a packet
                    sock_clear_input_buffer();
                    ret = sock_recv();

                    // 2. parse packet
                    packet_parse(sock_get_in_buf());
                    // packet_print_struct();

                    // 3. verify correct payload
                    crc32_calc = crc_generate(
                        packet_get_payload(), 
                        packet_get_payload_size()
                    );

                    if(crc32_calc != packet_get_crc32())
                    {
                        // data is corrupted, go back to step 1.
                        printf("CRC32 mismatch, corrupted packet!");
                        continue;
                    }

                    // 4. Check if ACK packet (means end of transmission)
                    if ( strcmp(packet_get_payload(), "ACK") == 0 )
                    {
                        // if the ack was not received, that means something went
                        // wrong during transmission. we need to start over and go
                        // to when user enters command. 
                        printf("ACK Received!\nEOF\n");
                        break;
                    }

                    // 5. save payload to file
                    ret = file_write_chunk(
                        packet_get_payload(), 
                        packet_get_payload_size()
                    );

                    if(ret != packet_get_payload_size())
                    {
                        printf("ERROR: Not all bytes written to file.\n");
                    }

                    // 6. generate ACK packet.
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

                    // 7. send ACK packet
                    ret = sock_sendto(packet_get_buf(), packet_get_total_size(), false);

                }
                file_close();
                sock_disable_timeout();
                
                // print some stats on our new file
                file_open(cmd_params, 0);
                printf("%s size (bytes): %d\n", cmd_params, file_get_size());
                uint32_t crc32_calc = crc_generate_file(file_get_fp());
                printf("File CRC: %u\n", crc32_calc);

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