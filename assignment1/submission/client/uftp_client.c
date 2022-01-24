#include "../include/menu.h"
#include "../include/utils.h"

#include <stdio.h>

int main(int argc, char *argv[]) 
{

    if(argc != 3)
    {
        msg_bad_args_client();
        return 0;
    }

    char *ip_str = argv[1];
    char *port_str = argv[2];

    if(!is_ip(ip_str))
    {
        msg_invalid_ip(ip_str);
        msg_bad_args_client();
        return 0;
    }

    if(!is_port(port_str))
    {
        msg_invalid_port(port_str);
        msg_bad_args_client();
        return 0;
    }



    test_greeting(0);
    return 0;
}