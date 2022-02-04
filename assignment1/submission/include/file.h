#ifndef INC_FILE_
#define INC_FILE_

#include <stdio.h>
#include <stdint.h>

#define MAX_FILE_BUF_SIZE (600)

char *file_get_file_buf();
int file_open(char* name, bool mode);
int file_get_size();
uint32_t file_read_chunk(uint32_t chunk_size);
uint32_t file_write_chunk(char* buf, uint32_t chunk_size);
void file_close();
uint32_t file_get_fileptr_location();
void file_print_buf(uint32_t); // this fuction prints the entire file in chunks
FILE *file_get_fp();
void file_print_all();
void file_clear_buf();
void file_reset_fileptr();
#endif /*INC_FILE_*/