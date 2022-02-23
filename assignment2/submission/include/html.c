#include "html.h"

static const html_req_struct_t req_methods[_total_types] = 
{
    {"GET"     , get},
    {"HEAD"    , head},
    {"POST"    , post},
    {"PUT"     , put},
    {"DELETE"  , delete},
    {"CONNECT" , connect},
    {"OPTIONS" , options},
    {"TRACE"   , trace},
};

// needs pointer to payload
// needs size of payload
void html_parse_request()
{
    // 1. scan payload for first CRLF
    // 2. allocate memory for a string which contains n number of bytes to hold
    //    the request-line.
    // 3. point p_request_line to the new memory location. 
    // 4. memcpy n bytes from payload 
    return;
}
