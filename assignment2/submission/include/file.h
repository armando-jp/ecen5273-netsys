#ifndef INC_FILE_
#define INC_FILE_

#include <stdbool.h>
#include <stdio.h>

FILE *file_open(char* name, bool mode);
void file_close(FILE *fp);
char *file_read_all(FILE *fp, int *size);

#endif /* INC_FILE_ */