#ifndef INC_THREADING_
#define INC_THREADING_

#include <stdbool.h>
#include <stdint.h>
#include "http.h"

#define MAX_NUMBER_OF_THREADS (100)

typedef struct {
    int new_fd;
    uint32_t thread_id;
    char *p_payload;
    uint32_t payload_size;
    http_req_results_t *request;
} thread_args_t;

int threading_create_dispatcher(int new_fd);
int threading_create_worker(http_req_results_t *p_results);

#endif // INC_THREADING_