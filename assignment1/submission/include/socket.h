#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_

#define USE_TIMEOUT (1)
#define TIMEOUT_SEC (5)
#define MAX_IN_BUF_LEN (600)

// function declerations
char *sock_get_in_buf();
bool sock_init_udp_struct(char *port, char *ip, bool is_client);
void sock_free_udp_struct();
void sock_print_ip();
bool sock_create_socket();
int sock_sendto(char *buf, uint32_t length, bool is_client);

void socket_init_timeout();
void sock_enable_timeout();
void sock_disable_timeout();

int sock_bind();
int sock_recv();
void sock_print_last_msg();
void sock_close_socket();
void sock_clear_input_buffer();
#endif /*INC_SOCKET_H_*/