#ifndef INC_THREADING_
#define INC_THREADING_

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "packet.h"
#include "conf_parsing.h"
#define MAX_NUMBER_OF_THREADS (100)

typedef struct {
    int fd_client;
    uint32_t thread_id;
    conf_results_dfs_t conf;
} thread_args_t;

typedef struct {
    int fd_client;
    uint32_t thread_id;
    Packet pkt;
} worker_thread_args_t;

int threading_create_dispatcher(int fd_client, conf_results_dfs_t conf);
int threading_create_worker(worker_thread_args_t *p_results, pthread_t *id);

#endif // INC_THREADING_