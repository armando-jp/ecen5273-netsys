#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_


// function declerations
bool sock_init_udp_struct(char *port, char *ip);
void sock_free_udp_struct();
void sock_print_ip();
bool sock_create_socket();
int sock_sendto(char *buf, uint32_t length);

#endif /*INC_SOCKET_H_*/