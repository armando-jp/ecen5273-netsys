#ifndef INC_HTTP_
#define INC_HTTP_

#include <stdbool.h>

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

typedef enum {
    http_1v0,
    http_1v1,
    _total_version_types
} http_req_version_t;

// holds the parsed message contents
// parsed into the three main components
typedef struct {
    char *p_request_line;
    char *p_request_headers;
    char *p_request_message;
} http_req_message_t;

// holds the important results needed for crafting responses.
// a data structure like this is what should be passed to a worker thread.
typedef struct {
    // request-line
    http_req_method_t req_method;
    http_req_version_t req_version;
    char *p_request_uri;

    // request-header
    bool keep_alive;
    int content_length;

    // request-body
    char *p_request_body;

} http_req_results_t;

http_req_results_t *http_parse_request(char *buf, int buf_size);

#endif // INC_HTTP_