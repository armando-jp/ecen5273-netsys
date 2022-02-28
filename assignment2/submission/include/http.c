#include "http.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

const http_req_method_strct_t http_req_method_types[] =
{
    {get, "GET"},
    {head, "HEAD"},
    {post, "POST"},
    {put, "PUT"},
    {delete, "DELETE"},
    {connect, "CONNECT"},
    {options, "OPTIONS"},
    {trace, "TRACE"},
};

const http_req_version_struct_t http_req_version_types[] =
{
    {http_1v0, "HTTP/1.1"},
    {http_1v1, "HTTP/1.0"},
};

// returns pointer to struct on success
// returns NULL otherwise (not a valid command)
http_req_results_t *http_parse_request(char *buf, int buf_size)
{
    int ret;
    http_req_results_t *p_results = (http_req_results_t*) malloc(sizeof(http_req_results_t));

    printf("===GOING TO PRINT TOKENS\n");

    ret = http_prase_msg(buf, p_results);
    if(ret == -1)
    {
        printf("Error in parse msg\r\n");
    }



    // http_hex_dump(buf, buf_size);

    // PRASE THE REQUEST LINE
    // char *p_token = strsep(&buf, "\r\n");

    // printf("REQ LINE: %s\r\n", p_req_line);
    // http_parase_req_line(p_req_line, p_results);

    printf("COMMAND: %d, URI: %s, VERSION: %d\r\n", p_results->req_method, p_results->p_request_uri, p_results->req_version);
    printf("CONNECTION: %d\r\n", p_results->keep_alive);
    // // PRASE THE HEADER FIELDS
    // printf("BUF: %s\r\n", buf);

    // if(strstr(buf, "\\r\\n\\r\\n") == 0)
    // {
    //     printf("FOUND \\r\\n\\r\\n\r\n");
    // }
    // else
    // {
    //     printf("DID NOT FIND \\r\\n\\r\\n\r\n");
    // }

    
    // while((p_token = strtok(buf, "\\r\\n\\r\\n")) != NULL)
    // {
    //     printf("PRINTING TOKEN: %s\r\n", p_token);
    // }
    // printf("PRINTING THE HEADER FIELDS: %s\r\n", p_token);

    // //DISPLAY PAYLOAD
    // p_token = strsep(&buf, "\\r\\n\\r\\n");
    // printf("DISPLAYING PAYLOAD: %s\r\n", p_token);

    // 
    // while(token)
    // {
    //     printf("%s", token);
    //     token = strsep(&buf, "\r\n");
    // }
    printf("===DONE PRINTING TOKENS\n");

    // Now process each token.
    // We know the first token is going to be the Request-Line
    // The second line up until we get the blank, will be the Request header
    // fields
    // The final portion (if any) will be the request-message field.
    // Iterate through these and toggle/set the appropriate parameters in the
    // http_req_results_t struct. This will be handed back to the worker thread.
    // The worker thread will use that information to craft it's response. (AKA,
    // call a different function to create the response by passing the
    // http_req_results_t variable) 
    return NULL;
}

void http_parase_req_line(char *buf, http_req_results_t *p_results)
{
    if(buf == NULL)
    {
        printf("INVALID POINTER PASSED TO HTTP PARSE REQ LINE\r\n");
        return;
    }

    char *p_token = NULL;
    p_token = strtok(buf, " ");

    // the first token should be the method type
    for(int i = 0; i < _total_method_types; i++)
    {
        if(strcasecmp(p_token, http_req_method_types[i].text) == 0)
        {
            // printf("COMMAND: %s\r\n", http_req_method_types[i].text);
            p_results->req_method = http_req_method_types[i].method_enum;
            break;
        }
    }

    // the second token should be the request URI
    p_token = strtok(NULL, " ");
    if(p_token != NULL)
    {
        // printf("REQUEST URI: %s\r\n", p_token);
        memcpy(&p_results->p_request_uri, p_token, strlen(p_token));
    }
    
    // the third token should be the HTTP version
    p_token = strtok(NULL, " ");
    if(p_token != NULL)
    {
        for(int i = 0; i < _total_version_types; i++)
        {
            if(strcmp(p_token, http_req_version_types[i].text) == 0)
            {
                // printf("HTTP VERSION: %s\r\n", p_token);
                p_results->req_version = http_req_version_types[i].version_enum;
                break;
            }
        }
    }

    // should be NULL
    p_token = strtok(NULL, " ");
    if(p_token != NULL)
    {
        printf("FOUND ANOTHER TOKEN?: %s\r\n", p_token);
    }
    else
    {
        printf("PARSE SUCCESS!\r\n");
    }
}

void http_parse_header_lines(char *buf, http_req_results_t *p_results)
{
    char *token;
    char *sub_token;
    char *saveptr;

    token = strtok_r(buf, "\r\n", &saveptr);
    do
    {

        printf("TOKEN: %s\r\n", token);

        if(strstr(token, "Connection:") != NULL)
        {
            if(strstr(token, "Keep-Alive") != NULL)
            {
                p_results->keep_alive = true;
            }
            else
            {
               p_results->keep_alive = false; 
            }
        }
        // Check for content length

        sub_token = strtok(token, ":");
        do
        {
            printf("SUB-TOKEN: %s\r\n", sub_token);
        } while((sub_token = strtok(NULL, ":")) != NULL);
    } while((token = strtok_r(saveptr, "\r\n", &saveptr)) != NULL);
}

// parses packet into req_line, header field, and payload
// returns three pointers to the new locations.
int http_prase_msg(char *src_buf, http_req_results_t *p_results)
{
    char *p_req_line = NULL;
    char *p_header = NULL;
    char *p_payload = NULL;

    // PHASE 1: Split raw message into request line, header, and payload.
    // Split message and obtain request line
    char *p_token = strsep(&src_buf, "\r\n");
    if(p_token == NULL)
    {
        return -1;
    }
    p_req_line = p_token;

    // Check if there is a payload. 
    if(strstr(src_buf, "\\r\\n\\r\\n") != 0)
    {
        // there is no payload
        p_payload = NULL;
        p_header = src_buf;
    }
    else // there is a payload. First split the header and payload.
    {
        int i = 0;
        p_header = src_buf;
        p_payload = src_buf;

        while(src_buf[i] != 0)
        {
            if(src_buf[i] == 0x0D)
            {
                if(src_buf[i+1] == 0x0A)
                {
                    if(src_buf[i+2] == 0x0D)
                    {
                        if(src_buf[i+3] == 0x0A)
                        {
                            // terminate the string there
                            src_buf[i+2] = 0;
                            p_header = src_buf;
                            p_payload = &src_buf[i+4];
                        }
                    }
                }
            }
            i++;
        }
    }


    // PHASE 2: Extract data from message fields and update result variable.

    // Parse request line and update results variable.
    http_parase_req_line(p_req_line, p_results);

    // Parse header line(s) and update results variable.
    http_parse_header_lines(p_header, p_results);

    // Just copy payload into results payload field.
    memcpy(&p_results->p_request_payload, p_payload, strlen(p_payload));

    // printf("REQ LINE:\r\n%s\r\n", p_req_line);
    // printf("HEADER FIELD:\r\n%s\r\n", p_header);
    // printf("PAYLOAD: \r\n%s\r\n", p_payload);

    return 0;
}



void http_hex_dump(char *buf, uint32_t buf_size)
{
    printf("\r\nSTART HEX DUMP======\r\n");
    for(int i = 0; i < buf_size; i ++)
    {
        printf("0x%02X ", buf[i]);
    }
    printf("\r\nEND HEX DUMP======\r\n");
}