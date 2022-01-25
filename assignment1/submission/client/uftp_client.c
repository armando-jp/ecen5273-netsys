#include "../include/msg.h"
#include "../include/cli.h"
#include "../include/utils.h"
#include "../include/socket.h"

#include <stdio.h>

int main(int argc, char *argv[]) 
{
    // variables for user input processing
    int ret;
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
    // TODO: ERROR HANDLING
    ret = sock_init_udp_struct(port_str, ip_str, true);
    sock_create_socket();
    // sock_print_ip();
    ret = sock_sendto("Hello from Client!\n", 20);
    printf("Sent %d bytes to %s\n", ret, ip_str);
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
                break;

            case CMD_PUT:
                // perform put operation
                break;

            case CMD_DELETE:
                // perform delete operation
                break;

            case CMD_LS:
                // perform ls operation
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