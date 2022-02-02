#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_

#define MAX_IN_BUF_LEN (600)

// function declerations
char *sock_get_in_buf();
bool sock_init_udp_struct(char *port, char *ip, bool is_client);
void sock_free_udp_struct();
void sock_print_ip();
bool sock_create_socket();
int sock_sendto(char *buf, uint32_t length);

int sock_bind();
int sock_recv();
void sock_print_last_msg();
void sock_close_socket();
void sock_clear_input_buffer();
#endif /*INC_SOCKET_H_*/