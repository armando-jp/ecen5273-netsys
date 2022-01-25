#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_

#define MAX_IN_BUF_LEN (100)

// function declerations
bool sock_init_udp_struct(char *port, char *ip, bool is_client);
void sock_free_udp_struct();
void sock_print_ip();
bool sock_create_socket();
int sock_sendto(char *buf, uint32_t length);

int sock_bind();
int sock_recv();
void sock_print_last_msg();
void sock_close_socket();
#endif /*INC_SOCKET_H_*/