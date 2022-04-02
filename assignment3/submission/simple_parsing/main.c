#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "http.h"

char in_buffer2[1000]  = "GET http://netsys.cs.colorado.edu/ HTTP/1.1\r\nHost: netsys.cs.colorado.edu\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:98.0) Gecko/20100101 Firefox/98.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\nCookie: _ga=GA1.2.1924393814.1639172945\r\nUpgrade-Insecure-Requests: 1\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\n";
char in_buffer[1000] = "PUT http://netsys.cs.colorado.edu/ HTTP/1.1\r\nHost: netsys.cs.colorado.edu\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:98.0) Gecko/20100101 Firefox/98.0\r\nContent-Length: 23\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\nCookie: _ga=GA1.2.1924393814.1639172945\r\nUpgrade-Insecure-Requests: 1\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\nHI I AM THE PAYLOAD WOO\r\n";

// void http_get_req(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end);
// void http_get_headers(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end);
// void http_get_header_values(char *p_buffer, uint16_t buffer_size, uint16_t *payload_size);
// void http_get_payload(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end, uint16_t payload_size);

int main()
{
    http_req_results_t *p_results = http_parse_request(in_buffer, 1000);
    if(p_results == NULL)
    {
        printf("something went wrong\r\n");
    }
    else
    {
        http_display_parsing_results(p_results);  
    }

    free(p_results);

    return 0;
}

// void http_get_req(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end)
// {
//     // assume req line starts from index 0.
//     *p_start = 0;

//     for(int i = *p_start; i < buffer_size; i++)
//     {
//         if(p_buffer[i] == '\r')
//         {
//             if(i+1 < buffer_size)
//             {
//                 if(p_buffer[i+1] == '\n');
//                 {
//                     *p_end = i;
//                     return;
//                 }
//             }
//         }
//     }
// }

// void http_get_headers(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end)
// {
//     // assume p_start is where req line ended (p_end)
//     *p_start = (*p_end) + 1;

//     for(int i = *p_start; i < buffer_size; i++)
//     {
//         if(p_buffer[i] == '\r')
//         {
//             if(i+1 < buffer_size)
//             {
//                 if(p_buffer[i+1] == '\n');
//                 {
//                     if(i+2 < buffer_size)
//                     {
//                         if(p_buffer[i+2] == '\r')
//                         {
//                             if(i+3 < buffer_size)
//                             {
//                                 if(p_buffer[i+3] == '\n')
//                                 {
//                                     *p_end = i+3;
//                                     return;
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

// // pass variable containing headers
// void http_get_header_values(char *p_buffer, uint16_t buffer_size, uint16_t *payload_size)
// {
//     char p_buffer_cpy[buffer_size];
//     memcpy(p_buffer_cpy, p_buffer, buffer_size);

//     char *save_ptr = NULL;
//     char *line = NULL;
//     char *val = NULL;
//     int i = 1;

//     line = strtok_r(p_buffer_cpy, "\r\n", &save_ptr);
//     while(line != NULL)
//     {
//         printf("header %d = %s\r\n", i, line);
//         // start: add logic here to identify special headers and save their value
//         val = strtok(line, ": ");
//         printf("\tkey: %s\r\n", val);
//         if(strstr(val, "Content-Length") != NULL || strstr(val, "content-length") != NULL || strstr(val, "Content-length") != NULL)
//         {
//             printf("found content length\r\n");
//             val = strtok(NULL, "\r\n");
//             printf("\tval: %s\r\n", val);
//             *payload_size = strtol(val, NULL, 10);
//         }
//         else
//         {
//             val = strtok(NULL, "\r\n");
//             printf("\tval: %s\r\n", val);
//         }
//         // end

//         line = strtok_r(NULL, "\r\n", &save_ptr);
//         i++;
//     }
// }


// void http_get_payload(char *p_buffer, uint16_t buffer_size, uint16_t *p_start, uint16_t *p_end, uint16_t payload_size)
// {
//     *p_start = (*p_end)+1;
//     while(*p_start == '\r' || *p_start == '\n')
//     {
//         *p_start = *p_start + 1;
//     }
//     *p_end = *p_start + payload_size;
// }