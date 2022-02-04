#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for close


#include "socket.h"

static int status;


static char ip4[INET_ADDRSTRLEN]; // space to hold IPv4 strings4

static struct addrinfo hints;
static struct addrinfo *servinfo = NULL; // points to results from getaddrinfo()
static int sockfd; // socket file descriptor
static struct addrinfo *ptr = NULL;

// server variables
static struct sockaddr_storage their_addr;
char in_buf[MAX_IN_BUF_LEN];
int numbytes;
socklen_t addr_len;



/*******************************************************************************
* Utility Functions
*******************************************************************************/
// prints the IP address generated in the servinfo struct
char *sock_get_in_buf()
{
    return in_buf;
}

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

// get sockaddr, IPv4 or IPv6:
void *sock_get_in_addr(struct sockaddr *sa)
{
    // IPv4
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    // IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Prints the last received message 
void sock_print_last_msg()
{
    printf("server: packet is %d bytes long\n", numbytes);
    in_buf[numbytes] = '\0';
    printf("server: packet contains \"%s\"\n", in_buf);
}

// clears the input buffer
void sock_clear_input_buffer()
{
    memset(in_buf, 0, sizeof(in_buf));
}

/*******************************************************************************
* Functions used for initialize/disable UDP interfaces
*******************************************************************************/
// this function first populates the "hints" struct.
// hints will be prepared differently depending 
bool sock_init_udp_struct(char *port, char *ip, bool is_client)
{
    memset(&hints, 0, sizeof(hints)); // ensure struct is empty
    if(is_client == true)
    {
        hints.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
        hints.ai_socktype = SOCK_DGRAM; // UDP socket
        status = getaddrinfo(ip, port, &hints, &servinfo);
    }
    else
    {
        hints.ai_family = AF_INET; // set to AF_INET6 to use IPv6
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP
        status = getaddrinfo(NULL, port, &hints, &servinfo);
    }
    

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

int sock_bind()
{
    for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) 
    {
        sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sockfd == -1) 
        {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (ptr == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
}

void sock_close_socket()
{
    close(sockfd);
}



/*******************************************************************************
* Functions SENDING and RECEIVING
*******************************************************************************/
// Attempts to receive (MAX_IN_BUF_LEN-1) bytes over a UDP connection.
// Stores result in (in_buf), number of bytes read (numbytes) message origin IP 
// (their_addr), and length of origin address (addr_len). 
//
// This is a BLOCKING CALL.
int sock_recv()
{
    addr_len = sizeof their_addr;
    numbytes = recvfrom(
        sockfd, 
        in_buf, 
        MAX_IN_BUF_LEN-1 , 
        0, 
        (struct sockaddr *)&their_addr, 
        &addr_len
    );
    if (numbytes == -1) 
    {
        perror("recvfrom");
        exit(1);
    }
    return numbytes;
}

int sock_sendto(char *buf, uint32_t length, bool is_client)
{
    if(is_client)
    {
        return sendto(sockfd, buf, length, 0, ptr->ai_addr, ptr->ai_addrlen);
    }

    return sendto(
        sockfd, 
        buf, 
        length, 
        0, 
        (struct sockaddr *)&their_addr,
        sizeof(their_addr)
    );
}