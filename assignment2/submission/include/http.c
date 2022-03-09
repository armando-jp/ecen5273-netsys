#include "http.h"
#include "file.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <regex.h> 

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
http_req_results_t *http_parse_request(char *p_buf, int buf_size)
{
    http_req_results_t *p_results = NULL;
    p_results = (http_req_results_t*) malloc(sizeof(http_req_results_t));
    memset(p_results->p_request_payload, 0, 1000);
    p_results->p_request_uri[0] = '\0';
    p_results->content_length = 0;
    p_results->fd_connection = 0;
    p_results->keep_alive = 0;
    p_results->p_request_payload[0] = '\0';
    p_results->req_method = 0;
    p_results->req_version = 0;
    p_results->thread_idx = 0;
    p_results->dp_thread_idx = 0;

    if(http_parase_msg(p_buf, p_results) == -1)
    {
        printf("Error in parse msg\r\n");
        free(p_results);
        return NULL;
    }

    return p_results;
}

// parses packet into req_line, header field, and payload
// returns three pointers to the new locations.
int http_parase_msg(char *buffer, http_req_results_t *p_results)
{
    char *p_req_line = NULL;
    char *p_header = NULL;
    char *p_payload = NULL;

    // 1. split message and obtain request line
    char *p_token = strsep(&buffer, "\r\n");
    if(p_token == NULL)
    {
        printf("Error splitting req line from header\r\n");
        return -1;
    }
    p_req_line = p_token;
    // printf("p_req_line: %s\r\n", p_req_line);

    // 2. get header and payload (if present)
    if(strstr(buffer, "\r\n\r\n") == NULL)
    {
        // there is no payload
        printf("NO PAYLOAD FOUND\r\n");
        p_payload = NULL;
        p_header = buffer;
        p_results->content_length = 0;
    }
    else // there is a payload. First split the header and payload.
    {
        int i = 0;
        printf("PAYLOAD FOUND\r\n");
        while(buffer[i] != 0)
        {
            if(buffer[i] == 0x0D)
            {
                if(buffer[i+1] == 0x0A)
                {
                    if(buffer[i+2] == 0x0D)
                    {
                        if(buffer[i+3] == 0x0A)
                        {
                            // terminate the string there
                            buffer[i+2] = 0;
                            buffer[i+3] = 0;

                            p_header = buffer;
                            p_payload = &buffer[i+4];
                        }
                    }
                }
            }
            i++;
        }
    }

    // Parse request line and update results variable.
    if(http_parase_req_line(p_req_line, p_results) == -1)
    {
        printf("ERROR: http_parse_req_line()\r\n");
        return -1;
    }

    // Parse header line(s) and update results variable.
    if(http_parse_header_lines(p_header, p_results) == -1)
    {
        printf("ERROR: http_parse_header_lines()\r\n");
        return -1;
    }

    // Just copy payload into results payload field.
    if(p_payload != NULL)
    {
        memcpy(&p_results->p_request_payload, p_payload, strlen(p_payload));
    }

    return 0;
}

int http_parase_req_line(char *buf, http_req_results_t *p_results)
{
    if(buf == NULL)
    {
        printf("INVALID POINTER PASSED TO HTTP PARSE REQ LINE\r\n");
        return -1;
    }
printf("PARSING THIS LINE: %s\r\n", buf);
    char *p_token = NULL;
    char *saveptr = NULL;
    p_token = strtok_r(buf, " ", &saveptr);
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
    printf("BUF @URI: %s\r\n", buf);
    p_token = strtok_r(NULL, " ", &saveptr);
    if(p_token != NULL)
    {
        printf("REQUEST URI [PARSING REQ LINE]: %s\r\n", p_token);
        memcpy(&p_results->p_request_uri, p_token, strlen(p_token));
        p_results->p_request_uri[strlen(p_token)] = '\0';
    }
    else
    {
        printf("http_parase_req_line: FAILED TO GET REQ URI\r\n");
        return -1;
    }
    
    // the third token should be the HTTP version
    printf("BUF @HTTP VERSION: %s\r\n", buf);
    p_token = strtok_r(NULL, " ", &saveptr);
    if(p_token != NULL)
    {
        printf("HTTP VERSION: %s\r\n", p_token);
        for(int i = 0; i < _total_version_types; i++)
        {
            if(strstr(p_token, http_req_version_types[i].text) == 0)
            {
                printf("HTTP VERSION: %s\r\n", p_token);
                p_results->req_version = http_req_version_types[i].version_enum;
                break;
            }
        }
    }
    else
    {
        printf("http_parase_req_line: DID NOT FIND HTTP VERSION\r\n");
        return -1;
    }

    // should be NULL
    p_token = strtok_r(NULL, " ", &saveptr);
    if(p_token != NULL)
    {
        printf("PARSE REQ LINE FOUND ANOTHER TOKEN?: %s\r\n", p_token);
        return -1;
    }

    printf("PARSE REQUEST LINE SUCCESS!\r\n");
    return 0;
}

int http_parse_header_lines(char *buf, http_req_results_t *p_results)
{
    char *token = NULL;
    char *sub_token = NULL;
    char *saveptr = NULL;
    // char *saveptr2 = NULL;

    token = strtok_r(buf, "\r\n", &saveptr);
    if(token == NULL)
    {
        printf("http_parse_header_lines: DID NOT FIND ANY HEADER LINES\r\n");
        return -1;
    }
    do
    {
        // Check if the header is Connection
        if(strstr(token, "Connection:") != NULL)
        {
            if(strcasestr(token, "Keep-Alive") != NULL)
            {
                p_results->keep_alive = true;
            }
            else
            {
               p_results->keep_alive = false; 
            }
        }
        // Check for header is Content length
        else if(strstr(token, "Content-Length:") != NULL)
        {
            // get the subtoken
            char *end = NULL;
            sub_token = strtok(token, ":");
            do
            {
                if((p_results->content_length = strtol(sub_token, &end, 10)) != 0)
                {
                    break;
                }
            } while ((sub_token = strtok(NULL, ":")) != NULL);
        }
        else
        {
            sub_token = strtok(token, ":");
        }
   
        do
        {
            // printf("SUB-TOKEN: %s\r\n", sub_token);
        } while((sub_token = strtok(NULL, ":")) != NULL);
    } while((token = strtok_r(saveptr, "\r\n", &saveptr)) != NULL);

    return 0;
}

char *http_create_response(http_req_results_t *p_results, int *size)
{
    uint32_t buf_offset = 0;
    uint32_t file_size = 0;
    char *file_buf = NULL;
    char* buffer = (char*)malloc(sizeof(char) * MAX_HTTP_RESPONSE);

    // create the start line
    for(int i = 0; i < _total_version_types; i++)
    {
        if(p_results->req_version == http_req_version_types[i].version_enum)
        {
            buf_offset += sprintf((buffer+buf_offset), "%s ", http_req_version_types[i].text);
            break;
        }
    }
    buf_offset += sprintf((buffer+buf_offset), "%s", "200 OK\r\n");

    // get payload if there is one
    // we do this before writing header so we can get the content-length field
    // ready in advance.
    if(p_results->req_method == get || p_results->req_method == post)
    {

        //open file here to be copied into payload
        FILE *fp = NULL;
        if((p_results->p_request_uri[0]) == '\0')
        {
            printf("ERROR: opening request URI. Got '\\0'\r\n");
        }
        else if(strcmp("/", p_results->p_request_uri) == 0)
        {
            fp = file_open("./www/index.html", 0);
        }
        else
        {
            char uri[200];
            sprintf(uri, "./www%s", p_results->p_request_uri);
            printf("DP: %d WT %d: ATTEMPTING TO OPEN URI: %s\r\n", p_results->dp_thread_idx, p_results->thread_idx, uri);
            fp = file_open(uri, 0);
        }  

        if(fp == NULL)
        {
            if(buffer != NULL)
            {
                free(buffer);
            }
            
            printf("ERROR: Opening requested file: %s\r\n", p_results->p_request_uri);
            return NULL;
        }

        uint32_t num_bytes_read;
        file_buf = file_read_all(fp, &file_size, &num_bytes_read);
        printf("Read %d bytes of %d\r\n", num_bytes_read, file_size);
        if(file_buf == NULL)
        {
            file_close(fp);
            if(buffer != NULL)
            {
                free(buffer);
            }
            return NULL;
        }
        file_close(fp);
    }

    // handle content types here
    if((p_results->p_request_uri[0] == 47) && (p_results->p_request_uri[1] == '\0'))
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: text/html\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".html") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: text/html\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".png") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: image/png\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".gif") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: image/gif\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".txt") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: image/plain\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".jpg") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: image/jpg\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".css") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: text/css\r\n");
    }
    else if(strstr(p_results->p_request_uri, ".js") != NULL)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Type: application/javascript\r\n");
    }
    else
    {
        printf("FAILED TO DETERMINE CONTENT TYPE OF %d\r\n", p_results->p_request_uri[0]);
    }
    
    // CREATE MESSAGE HEADERS 
    // if(p_results->keep_alive)
    // {
    //     buf_offset += sprintf((buffer+buf_offset), "Connection: Keep-Alive\r\n");
    // }
    if(file_size > 0)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Length: %d\r\n", file_size);
    }

    // WRITE PAYLOAD IF THERE IS ONE
    if(p_results->req_method == get || p_results->req_method == post)
    {
        // insert blank line
        buf_offset += sprintf((buffer+buf_offset), "\r\n");

        if(p_results->req_method == post)
        {
            buf_offset += sprintf((buffer+buf_offset), "<html><body><pre><h1>%s</h1></pre>", p_results->p_request_payload);
        }

        for(uint32_t i = 0; i < file_size; i++)
        {
            buffer[i + buf_offset] = file_buf[i];
        }
        buf_offset += file_size;

        if(file_buf != NULL)
        {
            free(file_buf);
        }

    }

    *size = buf_offset; 
    return buffer;
}

void http_hex_dump(char *buf, uint32_t buf_size)
{
    printf("\r\nSTART HEX DUMP======\r\n");
    for(uint32_t i = 0; i < buf_size; i ++)
    {
        printf("[%d] 0x%02X\r\n", i, buf[i]);
    }
    printf("\r\nEND HEX DUMP======\r\n");
}