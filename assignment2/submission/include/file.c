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

#include "http.h"

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
    if(name == NULL)
    {
        printf("file name is void\r\n");
        return NULL;
    }

    FILE *fp;
    if(mode == 0)
    {
        fp = fopen(name, "r");
    }
    else
    {
        fp = fopen(name, "w+");
    }

    if(fp == NULL)
    {
        perror("file_open:");
        printf("FAILED TO OPEN FILE %s\r\n", name);
        http_hex_dump(name, strlen(name));
        return NULL;
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
char *file_read_all(FILE *fp, uint32_t *size, uint32_t *num_bytes_read)
{
    char *buffer;
    long numbytes;
    uint32_t ret;

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
    ret = fread(buffer, sizeof(char), numbytes, fp);

    if(ret != numbytes)
    {
        printf("Failed to read all of file!\r\n");
    }
    // file_close(fp);
    *num_bytes_read = ret;
    *size = numbytes;
    return buffer;
}