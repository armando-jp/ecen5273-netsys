#include <stdio.h>
#include "menu.h"

static char *txt_bad_args_client = "Invalid arguments!\n\
Usage: client [ip_address] [port_number]\n";


void test_greeting(int selection)
{
    if (selection == 0)
    {
        printf("%s\n\r", CLIENT_GREETING);
    }
    else if (selection == 1)
    {
        printf("%s\n\r", SERVER_GREETING);
    }
    else
    {
        printf("HI GENERIC\n\r");
    }
}


void msg_bad_args_client()
{
    printf("%s", txt_bad_args_client);
}

void msg_invalid_ip(char *ip_str)
{
    printf("%s is an invalid IP address!\n", ip_str);
}

void msg_invalid_port(char *port_str)
{
    printf("%s is an invalid port!\n", port_str);
}