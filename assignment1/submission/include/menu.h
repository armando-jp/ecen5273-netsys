#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "cli.h"

#define CLIENT_GREETING ("HELLO from CLIENT")
#define SERVER_GREETING ("HELLO from SERVER")
#define MAX_USER_INPUT (50)

// function declerations
void test_greeting(int);
void msg_bad_args_client();
void msg_invalid_port(char *port_str);
void msg_invalid_ip(char *ip);
void msg_display_main_menu();
#endif /*INC_MENU_H_*/