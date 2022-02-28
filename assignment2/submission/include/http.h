#ifndef INC_HTTP_
#define INC_HTTP_

#include <stdbool.h>
#include <stdint.h>

#define MAX_REQUEST_PAYLOAD (1000)
#define MAX_REQUEST_URI (100)


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
    http_req_version_t version_enum;
    char *text;
} http_req_version_struct_t;



// HTTP header vars


typedef struct {
    bool connection_type; // true for keep-alive
} http_req_header_struct_t;

// holds the important results needed for crafting responses.
// a data structure like this is what should be passed to a worker thread.
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

} http_req_results_t;

http_req_results_t *http_parse_request(char *buf, int buf_size);
int http_prase_msg(char *src_buf, http_req_results_t *p_results);
void http_parase_req_line(char *buf, http_req_results_t *p_results);
void http_parse_header_lines(char *buf, http_req_results_t *p_results);

void http_hex_dump(char *buf, uint32_t buf_size);

#endif // INC_HTTP_