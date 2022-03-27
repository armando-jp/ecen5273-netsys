#ifndef INC_THREADING_
#define INC_THREADING_

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "http.h"

#define MAX_NUMBER_OF_THREADS (100)

typedef struct {
    int new_fd;
    uint32_t thread_id;
} thread_args_t;

int threading_create_dispatcher(int new_fd);
int threading_create_worker(http_req_results_t *p_results, pthread_t *id);
int threading_create_worker_proxy(http_req_results_t *p_results, pthread_t *id);

#endif // INC_THREADING_