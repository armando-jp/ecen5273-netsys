#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"
#include <stdio.h>

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
    // sock_create_socket();
    sock_bind();
    sock_free_udp_struct();

    printf("listener: waiting to recvfrom...\n");
    sock_recv();
    sock_print_last_msg();

    sock_close_socket();

    return 0;
}