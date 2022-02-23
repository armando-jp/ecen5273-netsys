#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "socket.h"

/*******************************************************************************
 * Important variables
*******************************************************************************/
static int sockfd;
static int new_fd;
static struct addrinfo hints;
static struct addrinfo *p_servinfo;
static struct addrinfo *p_addrinfo;
static struct sockaddr_storage their_addr;
static socklen_t sin_size;
static struct sigaction sa;
static char s[INET6_ADDRSTRLEN];
static int rv;
static int yes = 1;


/*******************************************************************************
 * Utilities
*******************************************************************************/
static bool is_number(char *str)
{
    while(*str)
    {
        // if the character is not a number, return false
        if(!isdigit(*str))
        {
            return false;
        }
        // point to next character
        str++;
    }
    return true;
}

bool is_port(char *port_str)
{
    uint32_t num;

    // check if string is holding only a number or not.
    if(!is_number(port_str))
    {
        return false;
    }

    // convert substring to number
    num = atoi(port_str); 

    if(num >= 0 && num <= 65535)
    {
        return true;
    }

    return false;
}

static void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*******************************************************************************
 * Functions for opening/closing sockets
*******************************************************************************/
static void init_tcp()
{
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
}

int sock_bind_to_port(char * p_port)
{
    init_tcp();

    if((rv = getaddrinfo(NULL, p_port, &hints, &p_servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all results and bind to the first we can
    for(p_addrinfo = p_servinfo; p_addrinfo != NULL; p_addrinfo = p_addrinfo->ai_next)
    {
        if((sockfd = socket(p_addrinfo->ai_family, p_addrinfo->ai_socktype, p_addrinfo->ai_protocol)) == -1)
        {
            perror("server: socket\n");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt\n");
            return 1;
        }

        if(bind(sockfd, p_addrinfo->ai_addr, p_addrinfo->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind\n");
            continue;
        }

        break;
    }

    freeaddrinfo(p_servinfo);

    if(p_servinfo == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        return 1;
    }

    if(listen(sockfd, MAX_PENDING_CONNECTIONS) == -1)
    {
        perror("listen\n");
        return 1;
    }
    return 0;
}

void sock_close(int fd)
{
    close(fd);
}

int sock_wait_for_connection()
{
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if(new_fd == -1)
    {
        perror("accept");
        return 1;
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
    printf("server: got connection from %s\n", s);

    return 0;
}

/*******************************************************************************
 * Reading/Writing Functions
*******************************************************************************/
int sock_read(int new_fd, char *buf, uint32_t buf_size, int use_timeout)
{
    int numbytes = 0;
    // struct containing timeout info
    struct timeval tv = {
        .tv_sec = MAX_TIMEOUT_SEC,
        .tv_usec = 0
    };
    fd_set readfds, master; // delcare a read set

    if(use_timeout)
    {
        // initialize the timeout
        FD_ZERO(&master);
        FD_ZERO(&readfds);
        FD_SET(new_fd, &master);
        tv.tv_sec = MAX_TIMEOUT_SEC;

        // enable the timeout
        readfds = master;
        select(new_fd+1, &readfds, NULL, NULL, &tv);

        // perform the recv
        if(FD_ISSET(new_fd, &readfds))
        {
            if((numbytes = recv(new_fd, buf, buf_size, 0)) == -1)
            {
                perror("recv\n");
                return 1;
            }
            FD_CLR(sockfd, &readfds);
        }
        else // timeout occured
        {
            printf("Timeout!\n");
            return -1;
        }
    }
    else // not using timeouts
    {
        if((numbytes = recv(new_fd, buf, buf_size-1, 0)) == -1)
        {
            perror("recv\n");
            return 1;
        }
    }

    return numbytes;
}

/*******************************************************************************
 * Getter/Setter Functions
*******************************************************************************/
int sock_get_new_fd()
{
    return new_fd;
}

int sock_get_sockfd()
{
    return sockfd;
}