#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "socket.h"

// TODO: Remove error handling from here (all printf statements) and 
// relocate to msg.c/h

static int status;

static char ip4[INET_ADDRSTRLEN]; // space to hold IPv4 strings4

static struct addrinfo hints;
static struct addrinfo *servinfo = NULL; // points to results from getaddrinfo()
static int sockfd; // socket file descriptor
static struct addrinfo *ptr = NULL;

// initialize servinfo struct using IPv4/IPv6, UDP, and assign address of 
// local host to the socket struct.
bool sock_init_udp_struct(char *port, char *ip)
{
    memset(&hints, 0, sizeof(hints)); // ensure struct is empty
    hints.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    status = getaddrinfo(ip, port, &hints, &servinfo);
    if(status != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return false;
    }
    // servinfo now points to a linked list of 1 or more struct addrinfos
    return true;
}

void sock_free_udp_struct()
{
    freeaddrinfo(servinfo); // free linked list
}

// prints the IP address generated in the servinfo struct
void sock_print_ip()
{
    struct addrinfo *p;
    char ipstr[INET6_ADDRSTRLEN];

    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) // IPv4
        { 
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else // IPv6
        { 
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }
}

bool sock_create_socket()
{
    struct addrinfo *p;

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) 
        {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return false;
    }

    ptr = p;
}

int sock_sendto(char *buf, uint32_t length)
{
    return sendto(sockfd, buf, length, 0, ptr->ai_addr, ptr->ai_addrlen);
}