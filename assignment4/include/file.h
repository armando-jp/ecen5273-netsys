#ifndef INC_FILE_
#define INC_FILE_

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*******************************************************************************
 * Utility functions
*******************************************************************************/
// returns file size in bytes
int file_get_size(FILE *fp);

/*******************************************************************************
 * Functions for OPENING and CLOSING files
*******************************************************************************/
FILE *file_open(char* name, bool mode);
FILE *file_open_create(char *name, bool mode);
void file_close(FILE *fp);

/*******************************************************************************
 * Functions for READING and WRITING files
*******************************************************************************/
uint32_t file_read(char *buffer, FILE *fp, uint32_t bytes);
uint32_t file_write(char* buffer, FILE *fp, uint32_t bytes);

/*******************************************************************************
 * Functions for DELETING files
*******************************************************************************/
int file_delete(char * file_name);

/*******************************************************************************
 * Functions for managing directory.
*******************************************************************************/
DIR *file_open_dir(const char *dir_name);
#endif /*INC_FILE_*/