#include "file.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>

/*******************************************************************************
 * Utility functions
*******************************************************************************/
// returns file size in bytes
int file_get_size(FILE *fp)
{
    int file_size = 0;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

/*******************************************************************************
 * Functions for OPENING and CLOSING files
*******************************************************************************/
FILE *file_open(char* name, bool mode)
{
    FILE *fp;
    if(mode == 0)
    {
        fp = fopen(name, "r");
    }
    else
    {
        fp = fopen(name, "w+");
    }

    return fp;
}

void file_close(FILE *fp)
{
    fclose(fp);
}

/*******************************************************************************
 * Functions reading files
*******************************************************************************/
char *file_read_all(FILE *fp, int *size)
{
    char *buffer;
    long numbytes;

    if(fp == NULL)
    {
        return NULL;
    }

    // Get size of file
    numbytes = file_get_size(fp);

    // create sufficient memory for buffer to hold file
    buffer = (char*)calloc(numbytes, sizeof(char));
    if(buffer == NULL)
    {
        return NULL;
    }

    // copy all contents into buffer
    fread(buffer, sizeof(char), numbytes, fp);
    // file_close(fp);

    *size = numbytes;
    return buffer;
}