#include "http.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// returns pointer to struct on success
// returns NULL otherwise (not a valid command)
http_req_results_t *http_parse_request(char *buf, int buf_size)
{
    printf("===GOING TO PRINT TOKENS\n");
    char *token = strsep(&buf, "\r\n");
    while(token)
    {
        printf("%s\n", token);
        token = strsep(&buf, "\r\n");
    }
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
