#ifndef INC_MENU_H_
#define INC_MENU_H_


#define CLIENT_GREETING ("HELLO from CLIENT")
#define SERVER_GREETING ("HELLO from SERVER")



// function declerations
void test_greeting(int);
void msg_bad_args_client();
void msg_invalid_port(char *port_str);
void msg_invalid_ip(char *ip);
#endif /*INC_MENU_H_*/