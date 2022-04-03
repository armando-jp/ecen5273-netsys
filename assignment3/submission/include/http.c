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

// returns pointer to struct on success MALLOCED
// returns NULL otherwise (not a valid command)
http_req_results_t *http_parse_request(char *p_buf, int buf_size)
{
    if(p_buf == NULL)
    {
        return NULL;
    }
    // Create variables for parsing
    uint16_t idx_start = 0;
    uint16_t idx_end = 0;
    char req[100];
    char header[1000];
    char payload[30000];
    memset(req, 0, 100);
    memset(header, 0 , 1000);
    memset(payload, 0, 30000);

    // Create HTTP struct
    http_req_results_t *p_results = NULL;
    p_results = (http_req_results_t*) malloc(sizeof(http_req_results_t));
    if(p_results == NULL)
    {
        printf("FAILED TO MALLOC FOR http_req_results_t\r\n");
        return NULL;
    }

    // Initialize HTTP struct
    //memset(p_results->p_request_payload, 0, MAX_REQUEST_PAYLOAD);
    p_results->p_request_uri[0] = '\0';
    p_results->content_length = 0;
    p_results->fd_client = 0;
    p_results->keep_alive = 0;
    //p_results->p_request_payload[0] = '\0';
    p_results->req_method = 0;
    p_results->req_version = 0;
    p_results->thread_idx = 0;
    p_results->dp_thread_idx = 0;
    p_results->p_original_http_request[0] = '\0';
    p_results->p_request_host[0] = '\0';
    p_results->original_http_request_size = 0;
    p_results->port = 0;
    p_results->actual_content_length = 0;

    // (1) get req line
    if(http_get_req(p_buf, buf_size, &idx_start, &idx_end, p_results) != 0)
    {
        printf("http_parse_request: Error in http_get_req\r\n");
        free(p_results);
        return NULL;
    }
    
    // (2) get header lines
    if(http_get_headers(p_buf, buf_size, &idx_start, &idx_end, p_results) != 0)
    {
        printf("http_parse_request: Error in http_get_headers\r\n");
        free(p_results);
        return NULL;
    }

    // (5) get payload (if present)
    if(p_results->content_length > 0)
    {
        http_get_payload(p_buf, buf_size, &idx_start, &idx_end, p_results);
    }

    // (6) Write copy of original HTTP request to HTTP struct.
    memcpy(p_results->p_original_http_request, p_buf, buf_size);
    p_results->original_http_request_size = buf_size;

    return p_results;
}

// parses packet into req_line, header field, and payload
// returns three pointers to the new locations.
int http_parase_msg(char *buffer, http_req_results_t *p_results)
{
    char *p_req_line = NULL;
    char *p_header = NULL;
    char *p_payload = NULL;

    /***************************************************************************
     * PHASE 1: Split HTTP request into 3 fields: Request, Header, and Payload
     **************************************************************************/
    // 1. split message and obtain request line
    char *p_token = strsep(&buffer, "\r\n");
    if(p_token == NULL)
    {
        printf("http_parase_msg: ERROR SPLITING REQ LINE\r\n");
        return -1;
    }
    p_req_line = p_token;

    // 2. get header and payload (if present)
    if(strstr(buffer, "\r\n\r\n") == NULL)
    {
        // there is no payload
        printf("http_parase_msg: NO PAYLOAD FOUND\r\n");
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

    /***************************************************************************
     * PHASE 2: Extract specific information from the three HTTP request fields.
     **************************************************************************/
    // (1) Prase request line
    if(http_parase_req_line(p_req_line, p_results) == -1)
    {
        printf("ERROR: http_parse_req_line()\r\n");
        return -1;
    }

    // (2) Parse header field
    if(http_parse_header_lines(p_header, p_results) == -1)
    {
        printf("ERROR: http_parse_header_lines()\r\n");
        return -1;
    }

    // (3) Copy payload (if there is one)
    // if(p_payload != NULL)
    // {
    //     memcpy(&p_results->p_request_payload, p_payload, MAX_REQUEST_PAYLOAD);
    // }

    return 0;
}

int http_parase_req_line(char *buf, http_req_results_t *p_results)
{
    char *p_token = NULL;
    char *saveptr = NULL;

    if(buf == NULL)
    {
        printf("http_parase_req_line: INVALID POINTER ARG\r\n");
        return -1;
    }

    // (1) Obtain the method type (GET, POST, etc.)
    printf("http_parase_req_line: parsing this line(CMD)->%s\r\n", buf);
    p_token = strtok_r(buf, " ", &saveptr);
    for(int i = 0; i < _total_method_types; i++)
    {
        if(strcasecmp(p_token, http_req_method_types[i].text) == 0)
        {
            p_results->req_method = http_req_method_types[i].method_enum;
            break;
        }
    }

    // (2) Get the request URI & service type & host
    p_token = strtok_r(NULL, " ", &saveptr);
    printf("http_parase_req_line: parsing this line(URI)->%s\r\n", p_token);
    if(p_token != NULL)
    {
        // Save the original request URI
        printf("REQUEST URI [PARSING REQ LINE]: %s\r\n", p_token);
        memcpy(&p_results->p_request_uri, p_token, strlen(p_token));
        p_results->p_request_uri[strlen(p_token)] = '\0';

        // Get service string (http or https...etc) and host.
        if(strstr(p_token, "http://") != NULL)
        {
            memcpy(&p_results->p_request_host, p_token+7, strlen(p_token)-7);
            p_results->p_request_host[strlen(p_token)-7] = '\0';
            memcpy(p_results->p_service, "http", strlen("http"));
        }
        else if(strstr(p_token, "https://") != NULL)
        {
            memcpy(&p_results->p_request_host, p_token+8, strlen(p_token)-8);
            p_results->p_request_host[strlen(p_token)-8] = '\0';
            memcpy(p_results->p_service, "https", strlen("https"));
        }
        else
        {
            // if there was not service in the url, then copy URI 
            memcpy(&p_results->p_request_host, &p_results->p_request_uri, strlen(p_results->p_request_uri));
            memcpy(p_results->p_service, "http", strlen("http"));
        }

        // check if there is a final '/' and replace it with '\0'. If not, then
        // that means the URI does not have it either and it needs to be added
        // (to the URI).
        char *ptr;
        if((ptr = strchr(p_results->p_request_host, '/')) != NULL)
        {
            ptr[0] = '\0';
        }
        else
        {
            p_results->p_request_uri[strlen(p_results->p_request_uri)] = '/';
            p_results->p_request_uri[strlen(p_results->p_request_uri) + 1] = '0';
        }

        //check if there is a ':' denoting a specific PORT.
        if((ptr = strchr(p_results->p_request_host, ':')) != NULL)
        {
            ptr[0] = '\0';
            p_results->port = atoi(ptr+1);
        }

    }
    else
    {
        printf("http_parase_req_line: FAILED TO GET REQ URI\r\n");
        return -1;
    }
    printf("GOT URI: %s\r\nGOT HOST: %s\r\nGOT SERVICE: %s\r\n", 
    p_results->p_request_uri, p_results->p_request_host, p_results->p_service);
    
    // (3) Get HTTP version
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

    // (4) Check for errors in HTTP message 
    // (extra tokens, things that should not be there, etc.)
    p_token = strtok_r(NULL, " ", &saveptr);
    if(p_token != NULL)
    {
        printf("http_parase_req_line: FOUND ANOTHER TOKEN?->%s\r\n", p_token);
        return -1;
    }

    printf("http_parase_req_line: SUCCESS!\r\n");
    return 0;
}

int http_parse_header_lines(char *buf, http_req_results_t *p_results)
{
    char *token = NULL;
    char *sub_token = NULL;
    char *saveptr = NULL;

    token = strtok_r(buf, "\r\n", &saveptr);
    if(token == NULL)
    {
        printf("http_parse_header_lines: DID NOT FIND ANY HEADER LINES\r\n");
        //return -1;
        return 0; // only for proxy app
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

/*******************************************************************************
 * (1) Write the 'start line'. 
 * (2) Get payload and size.
 * (3) Write the 'content-type' header.
 * (4) Write the 'content-length' header.
 * (5) Write the payload field.
 * (6) Return pointer to generated HTTP response message.
 ******************************************************************************/
char *http_create_response(http_req_results_t *p_results, int *size)
{
    uint32_t buf_offset = 0;
    uint32_t file_size = 0;
    char *file_buf = NULL;
    char* buffer = (char*)malloc(sizeof(char) * MAX_HTTP_RESPONSE);

    // (1) Create the start line
    for(int i = 0; i < _total_version_types; i++)
    {
        if(p_results->req_version == http_req_version_types[i].version_enum)
        {
            buf_offset += sprintf((buffer+buf_offset), "%s ", http_req_version_types[i].text);
            break;
        }
    }
    buf_offset += sprintf((buffer+buf_offset), "%s", "200 OK\r\n");

    // (2) Get payload (if there is one) and size.
    //     This is done before writing the header field in order to get the data
    //     needed for the 'content-length' header entry.
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

    // (3) Write the content type header field entry based on the file type.
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
    
    // (4) Write the 'content-length' header field entry (if there is a payload)
    if(file_size > 0)
    {
        buf_offset += sprintf((buffer+buf_offset), "Content-Length: %d\r\n", file_size);
    }

    // (5) Write the payload field (if there is a paylaod)
    if(p_results->req_method == get || p_results->req_method == post)
    {
        // insert blank line
        buf_offset += sprintf((buffer+buf_offset), "\r\n");

        if(p_results->req_method == post)
        {
            // buf_offset += sprintf((buffer+buf_offset), "<html><body><pre><h1>%s</h1></pre>", p_results->p_request_payload);
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

// new stuff
void http_display_parsing_results(http_req_results_t *p_results)
{
    if(p_results == NULL)
    {
        return;
    }
    printf("START: DISPLAY PARSING RESULTS\r\n");

    printf("REQUEST FIELDS\r\n");
    printf("\tMethod: %d\r\n", p_results->req_method);
    printf("\tURI: %s\r\n", p_results->p_request_uri);
    printf("\tVersion: %d\r\n", p_results->req_version);

    printf("HEADER FIELDS\r\n");
    printf("\tKeep-Alive: %d\r\n", p_results->keep_alive);
    printf("\tContent-Length: %d\r\n", p_results->content_length);
    printf("\tHost: %s\r\n", p_results->p_request_host);
    printf("\tPort: %d\r\n", p_results->port);

    // printf("PAYLOAD\r\n");
    // if(p_results->content_length > 0)
    // {
    //     printf("%s\r\n", p_results->p_request_payload);
    // }
    // else
    // {
    //     printf("\t<NO PAYLOAD>\r\n");
    // }
    
    printf("END: DISPLAY PARSING RESULTS\r\n");

}

int http_get_req(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end, http_req_results_t *p_results)
{
    // assume req line starts from index 0.
    *p_end = 0;
    *p_start = 0;

    // (1) get the starting location 
    for(int i = *p_start; i < buffer_size; i++)
    {
        if(p_buffer[i] == '\r')
        {
            if(i+1 < buffer_size)
            {
                if(p_buffer[i+1] == '\n')
                {
                    *p_end = i;
                    break;
                }
            }
        }
    }
    if(*p_end == 0)
    {
        return -1;
    }

    // (2) do a memcpy cause what we're about to do might be destructive
    uint16_t size_of_req = *p_end-*p_start;
    char req[size_of_req];
    memcpy(req, (p_buffer+*p_start), size_of_req);

    // (3) Obtain the method type (GET, POST, etc.)
    char *p_token = NULL;
    char *saveptr = NULL;

    p_token = strtok_r(req, " ", &saveptr);
    for(int i = 0; i < _total_method_types; i++)
    {
        if(strcasecmp(p_token, http_req_method_types[i].text) == 0)
        {
            p_results->req_method = http_req_method_types[i].method_enum;
            break;
        }
    }

    // (4) Get the request URI & service type
    p_token = strtok_r(NULL, " ", &saveptr);
    if(p_token != NULL)
    {
        // Save the original request URI
        memcpy(&p_results->p_request_uri, p_token, strlen(p_token));
        p_results->p_request_uri[strlen(p_token)] = '\0';

        // Get service string (http or https...etc)
        if(strstr(p_token, "https://") != NULL)
        {
            // memcpy(&p_results->p_request_host, p_token+8, strlen(p_token)-8);
            // p_results->p_request_host[strlen(p_token)-8] = '\0';
            strcpy(p_results->p_service, "https");
        }
        else
        {
            // memcpy(&p_results->p_request_host, p_token+7, strlen(p_token)-7);
            // p_results->p_request_host[strlen(p_token)-7] = '\0';
            strcpy(p_results->p_service, "http");
        }
    }
    else
    {
        printf("http_get_req: FAILED TO GET REQ URI\r\n");
        return -1;
    }
    return 0;
}

int http_get_headers(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end, http_req_results_t *p_results)
{
    // assume p_start is where req line ended (p_end)
    *p_start = (*p_end) + 1;
    *p_end = 0;

    // (1) find the beginning and end of the header section
    for(int i = *p_start; i < buffer_size; i++)
    {
        if(p_buffer[i] == '\r')
        {
            if(i+1 < buffer_size)
            {
                if(p_buffer[i+1] == '\n')
                {
                    if(i+2 < buffer_size)
                    {
                        if(p_buffer[i+2] == '\r')
                        {
                            if(i+3 < buffer_size)
                            {
                                if(p_buffer[i+3] == '\n')
                                {
                                    *p_end = i+3;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(*p_end == 0)
    {
        printf("http_get_headers: failed to find the beginning and end of the header section\r\n");
        return -1;
    }

    // (2) do a memcpy cause what we're about to do might be destructive
    uint16_t size_of_header = (*p_end-*p_start) + 1;
    char header[size_of_header];
    memcpy(header, (p_buffer+*p_start), size_of_header);
    header[size_of_header-1] = '\0';

    // (3) declare some useful variables for parsing header
    char *save_ptr = NULL;
    char *line = NULL;
    char *val = NULL;
    int i = 1;

    // (4) Iterate through all of the header key and value pairs. 
    //     If we find a pair we are interested in, we record that information in
    //     our results struct.
    line = strtok_r(header, "\r\n", &save_ptr);
    while(line != NULL)
    {
        printf("header %d = %s\r\n", i, line);
        // start: add logic here to identify special headers and save their value
        val = strtok(line, ": ");
        if(val == NULL)
        {
            break;
        }
        if(strstr(val, "Content-Length") != NULL || strstr(val, "content-length") != NULL || strstr(val, "Content-length") != NULL)
        {
            val = strtok(NULL, "\r\n");
            p_results->content_length = strtol(val, NULL, 10);
        }
        else if(strstr(val, "Keep-Alive") != NULL || strstr(val, "Keep-alive") != NULL || strstr(val, "keep-alive") != NULL)
        {
            val = strtok(NULL, "\r\n");
            p_results->keep_alive = true;
        }
        else if(strstr(val, "Host") != NULL || strstr(val, "host") != NULL)
        {
            val = strtok(NULL, "\r\n");
            // add 1 to val because we always get a space infront of the host name.
            memcpy(p_results->p_request_host, val+1, strlen(val)-1);
            p_results->p_request_host[strlen(val)-1] = '\0';
        }
        else
        {
            val = strtok(NULL, "\r\n");
        }
        // end

        line = strtok_r(NULL, "\r\n", &save_ptr);
        i++;
    }
    return 0;
}


void http_get_payload(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end, http_req_results_t *p_results)
{
    *p_start = (*p_end)+1;
    while(*p_start == '\r' || *p_start == '\n')
    {
        *p_start = *p_start + 1;
    }
    // *p_end = *p_start + p_results->content_length;
    // memcpy(p_results->p_request_payload, p_buffer+*p_start, (*p_end -
    // *p_start));
    *p_end = buffer_size - *p_start;
    p_results->actual_content_length = (*p_end - *p_start);
}