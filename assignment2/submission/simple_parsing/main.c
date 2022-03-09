#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_REQUEST_PAYLOAD (1000)
#define MAX_REQUEST_URI (100)
#define MAX_HTTP_RESPONSE (800000)

// HTTP method variables
typedef enum {
    get,
    head,
    post,
    put,
    delete,
    connect,
    options,
    trace,
    _total_method_types
} http_req_method_t;

typedef struct {
    http_req_method_t method_enum;
    char *text;
} http_req_method_strct_t;

// HTTP version variables
typedef enum {
    http_1v0,
    http_1v1,
    _total_version_types
} http_req_version_t;

typedef struct {
    // request-line
    http_req_method_t req_method;
    http_req_version_t req_version;
    char p_request_uri[MAX_REQUEST_URI];

    // request-header
    bool keep_alive;
    int content_length;

    // request-body
    char p_request_payload[MAX_REQUEST_PAYLOAD];

    // Socket FD connector
    int fd_connection;

    // other
    int thread_idx;
    int dp_thread_idx;

} http_req_results_t;

char *buff = "GET / HTTP/1.1\r\nHost: 198.59.7.11:60002\r\nConnection: keep-alive\r\nPragma: no-cache\r\nCache-Control: no-cache\r\nDNT: 1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: en-US,en;q=0.9\r\n\r\nI AM THE PAYLOAD";
char *buffer = NULL;


int main()
{
    http_req_results_t *p_results = NULL;
    p_results = (http_req_results_t *)malloc(sizeof(http_req_results_t));

    buffer = (char *)malloc(strlen(buff));
    memcpy(buffer, buff, strlen(buff));

    char *p_req_line = NULL;
    char *p_headers = NULL;
    char *p_payload = NULL;

    char *p_result = NULL;

    // // PHASE 1: Split raw message into request line, header, and payload.
    // // Get REQ LINE
    // char *p_token = strsep(&buffer, "\r\n");
    // if(p_token == NULL)
    // {
    //     printf("COULD NOT SPLIT TO GET REQ LINE\r\n");
    //     return -1;
    // }
    // p_req_line = p_token;

    // 1. CHECK IF PAYLOAD EXISTS. IF SO, SAVE THE POINTER.
    if((p_payload = strstr(buffer, "\r\n\r\n")) != NULL)
    {
        printf("buffer contains a payload section\r\n");
        //now extract the payload
        printf("PAYLOAD FOUND: %s\r\n", p_payload);
    }
    else
    {
        printf("buffer does not contain a payload\r\n");
    }

    // 2. SPLIT THE STRING BY \r\n. FIRST TIME WILL BE FOR REQ LINE
    //    SUBSEQUENT SPLITS WILL BE FOR HEADERS.
    while((p_result = strsep(&buffer, "\r\n")) != NULL)
    {
        // CHECK IF TOKEN IS EMPTY
        if(strlen(p_result) == 0)
        {
            continue;
        }


        printf("STRSTR GOT %s OF LENGTH %ld\r\n", p_result, strlen(p_result));

        if(strchr(p_result, ':') != NULL)
        {
            printf("this string contains ':' -> %s\r\n", p_result);
        }

        printf("\r\n");
    }

    return 0;
}