#include <stdio.h>
#include "menu.h"
#include "cli.h"

char user_input[MAX_USER_INPUT];

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

void msg_display_main_menu()
{
    printf("Main Menu\n");
    for(int i = 1; i <= NUM_COMMANDS; i++)
    {
        printf("%d. %s", i, get_help(i));
    }
}

// char *get_user_input()
// {
//     fgets(user_input, MAX_USER_INPUT, )
// }