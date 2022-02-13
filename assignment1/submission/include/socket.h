#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_

#define TIMEOUT_SEC (5)
#define MAX_IN_BUF_LEN (600)

/*******************************************************************************
* Utility Functions
*******************************************************************************/
char *sock_get_in_buf();
void sock_print_ip();
// void *sock_get_in_addr(sockaddr *sa);
void sock_print_last_msg();
void sock_clear_input_buffer();

/*******************************************************************************
* Functions used for initialize/disable UDP interfaces
*******************************************************************************/
bool sock_init_udp_struct(char *port, char *ip, bool is_client);
void sock_free_udp_struct();
bool sock_create_socket();
int sock_bind();
void sock_close_socket();

/*******************************************************************************
* Timeout Funcs
*******************************************************************************/
void socket_init_timeout();
void sock_enable_timeout();
void sock_disable_timeout();

/*******************************************************************************
* Functions SENDING and RECEIVING
*******************************************************************************/
int sock_recv();
int sock_sendto(char *buf, uint32_t length, bool is_client);

#endif /*INC_SOCKET_H_*/