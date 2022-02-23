#ifndef INC_HTML_
#define INC_HTML_

typedef enum {
    get,
    head,
    post,
    put,
    delete,
    connect,
    options,
    trace,
    _total_types
} html_req_method_t;

typedef struct {
    char *p_req_method_str;
    html_req_method_t req_method;
} html_req_struct_t;

typedef struct {
    char *p_request_line;
    char *p_request_headers;
    char *p_request_message;
} html_req_message_t;


void html_parse_request();

#endif // INC_HTML_