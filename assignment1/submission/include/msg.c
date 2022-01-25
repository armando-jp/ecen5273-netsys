#include <stdio.h>
#include "msg.h"

static char *txt_bad_args_client = "Invalid arguments!\n\
Usage: client [ip_address] [port_number]\n";

static char *txt_bad_args_server = "Invalid arguments!\n\
Usage: server [port_number]\n";


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

void msg_bad_command()
{
    printf("No matching command found/missing arguments.\n");
    printf("Please try again.\n\n");
}

void msg_app_closing()
{
    printf("Terminating client application.\n");
}

void msg_bad_args_server()
{
    printf("%s", txt_bad_args_server);
}