#include <stdio.h>

#include "./include/state_machine.h"
#include "./include/file.h"
#include "./include/html.h"
#include "./include/socket.h"
#include "./include/threading.h"


int main(int argc, char *argv[])
{
    // verify the correct number of arguments
    if(argc != 2)
    {
        printf("Invalid number of arguments.\n");
        printf("Usage:\n");
        printf("web_server [port]\n");
        return -1;
    }

    // validate port argument
    char *p_port = argv[1];
    if(!is_port(p_port))
    {
        printf("Invalid port parameter!\n");
        printf("Requirement: 0 <= port <= 65535");
        return -1;
    }

    printf("PORT: %s\n", p_port);


    printf("Binding to port %s\n", p_port);
    sock_bind_to_port(p_port);

    sm_server();

    printf("Closing connection\n");
    sock_close(sock_get_sockfd());


    return 0;
}