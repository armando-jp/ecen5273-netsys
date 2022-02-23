#ifndef INC_SOCKET_
#define INC_SOCKET_

#include <stdbool.h>
#include <stdint.h>

#define MAX_PENDING_CONNECTIONS (10)

int sock_bind_to_port(char * p_port);
int sock_wait_for_connection();

void sock_close(int fd);
bool is_port(char *port_str);
int sock_read(int new_fd, char *buf, uint32_t buf_size);
int sock_get_new_fd();
int sock_get_sockfd();

#endif // INC_SOCKET_