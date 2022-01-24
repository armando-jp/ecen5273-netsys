#ifndef INC_MSG_H_
#define INC_MSG_H_

#define CLIENT_GREETING ("HELLO from CLIENT")
#define SERVER_GREETING ("HELLO from SERVER")

// function declerations
void test_greeting(int);
void msg_bad_args_client();
void msg_invalid_port(char *port_str);
void msg_invalid_ip(char *ip);
void msg_bad_command();
void msg_app_closing();
#endif /*INC_MSG_H_*/