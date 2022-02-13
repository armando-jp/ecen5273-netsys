#ifndef INC_FILE_
#define INC_FILE_

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_FILE_BUF_SIZE (600)

/*******************************************************************************
 * Getter/Setter Functions
*******************************************************************************/
char *file_get_file_buf();
uint32_t file_get_fileptr_location();
FILE *file_get_fp();

/*******************************************************************************
 * Utility functions
*******************************************************************************/
int file_get_size();
void file_print_buf(uint32_t bytes);
void file_print_all(uint32_t chunk_size);
void file_clear_buf();
void file_reset_fileptr();
/*******************************************************************************
 * Functions for OPENING and CLOSING files
*******************************************************************************/
int file_open(char* name, bool mode);
void file_close();

/*******************************************************************************
 * Functions for READING and WRITING files
*******************************************************************************/
uint32_t file_read_chunk(uint32_t chunk_size);
uint32_t file_write_chunk(char* buf, uint32_t chunk_size);

/*******************************************************************************
 * Functions for DELETING files
*******************************************************************************/
int file_delete(char * file_name);

/*******************************************************************************
 * Functions for getting directory contents (LS)
*******************************************************************************/
extern int alphasort();
// static int file_select(struct direct *entry);
int file_get_ls();
void file_print_ls_buf(char *buf);

#endif /*INC_FILE_*/