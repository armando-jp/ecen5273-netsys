#ifndef INC_THREADING_
#define INC_THREADING_

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUMBER_OF_THREADS (100)

typedef struct {
    int new_fd;
    uint32_t thread_id;
} thread_args_t;

int threading_create_dispatcher(int new_fd);

#endif // INC_THREADING_