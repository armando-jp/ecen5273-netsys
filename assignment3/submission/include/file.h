#ifndef INC_FILE_
#define INC_FILE_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

FILE *file_open(char* name, bool mode);
void file_close(FILE *fp);
char *file_read_all(FILE *fp, uint32_t *size, uint32_t *num_bytes_read);

#endif /* INC_FILE_ */