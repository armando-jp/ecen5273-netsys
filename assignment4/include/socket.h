#ifndef INC_SOCKET_
#define INC_SOCKET_

#include <stdbool.h>
#include <stdint.h>

#define MAX_PENDING_CONNECTIONS (10)
#define MAX_TIMEOUT_SEC (10)

int sock_bind_to_port(char * p_port);
int sock_connect_to_host(char *host, char *port);
int sock_wait_for_connection(int sockfd);

void sock_close(int fd);
int sock_read(int new_fd, char *buf, uint32_t buf_size, int use_timeout);
int sock_send(int new_fd, char *buf, uint32_t buf_size);

#endif // INC_SOCKET_