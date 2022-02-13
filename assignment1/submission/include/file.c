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

#include "file.h"
#include "crc.h"


FILE *fp;
char file_buf[MAX_FILE_BUF_SIZE];
uint32_t file_size;
uint32_t file_pointer = 0;


/*******************************************************************************
 * Getter/Setter Functions
*******************************************************************************/
char *file_get_file_buf()
{
    return file_buf;
}

uint32_t file_get_fileptr_location()
{
    return file_pointer;
}

FILE *file_get_fp()
{
    return fp;
}

/*******************************************************************************
 * Utility functions
*******************************************************************************/
// returns file size in bytes
int file_get_size()
{
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

// prints file_buf contents as hex
void file_print_buf(uint32_t bytes)
{
    int i = 0;
    while(bytes)
    {
        printf("%02x ", file_buf[i] & 0xFF);
        bytes--;
        i++;
    }
}

// this fuction prints the entire file in chunks
void file_print_all(uint32_t chunk_size)
{
    uint32_t read = 0;
    uint32_t i = 0;
    uint32_t checksum = 0;
    uint32_t size = file_get_size();

    file_clear_buf();

    while(size)
    {
        read = file_read_chunk(chunk_size);
        i++;
        size -= read;


        printf("Iteration %d\n", i);
        printf("Read %d bytes\n", read);
        checksum = crc_generate(file_buf, read);
        printf("Checksum: %u\n", checksum);
        file_print_buf(read);
        printf("\n\n");

    }
}

void file_clear_buf()
{
    memset(file_buf, 0, sizeof(file_buf));
}

void file_reset_fileptr()
{
    fseek(fp, 0L, SEEK_SET);
}

/*******************************************************************************
 * Functions for OPENING and CLOSING files
*******************************************************************************/
// opens a file for reading
int file_open(char* name, bool mode)
{
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
        return -1;
    }

    return 0;
}

void file_close()
{
    fclose(fp);
}


/*******************************************************************************
 * Functions for READING and WRITING files
*******************************************************************************/
uint32_t file_read_chunk(uint32_t chunk_size)
{
    uint32_t total_read = 0;

    total_read = fread(
        file_buf,     // location of buffer
        1,            // size of each element to read
        chunk_size,   // number of elements to read
        fp            // input stream 
    );

    file_pointer += total_read;

    return total_read;
}

uint32_t file_write_chunk(char* buf, uint32_t chunk_size)
{
    uint32_t total_write = 0;

    total_write = fwrite(
        buf,          // location of buffer
        1,            // size of each element to write
        chunk_size,   // number of elements to write
        fp            // output stream 
    );

    file_pointer += total_write;

    return total_write;
}

/*******************************************************************************
 * Functions for DELETING files
*******************************************************************************/
int file_delete(char * file_name)
{
    return remove(file_name);
}

/*******************************************************************************
 * Functions for getting directory contents (LS)
*******************************************************************************/
static int file_select(const struct direct *entry)
{
    if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
    {
        return (0);
    }
    else
    {
        return (1);
    }
}

int file_get_ls()
{
    int count;
    int i;
    struct direct **files;

    file_clear_buf();

    if(!getcwd(file_buf, sizeof(file_buf)))
    {
        printf("Error getting pathname\n");
        return -1;
    }
    // printf("Pathname obtained: %s\n", file_buf);

    count = scandir(file_buf, &files, file_select, alphasort);

    // If no files found,make a non-selectable menu item
    if(count <= 0)
    {
        printf("No files in this directory\n");
        return -1;
    }

    // save file names info file_buf
    file_clear_buf();
    printf("Number of files = %d\n", count);
    for (i=0; i < count; ++i)
    {
        strcat(file_buf, files[i]->d_name);
        strncat(file_buf, " \n", 1);

        //printf("%s  ", files[i]->d_name);
    }
    // printf("\n");

    return strlen(file_buf);
}

void file_print_ls_buf(char *buf)
{
    // !!!
    //THIS FUNCTION IS DESTRUCTIVE. file_buf[] WILL BE PERMANENTLY MODIFIED.
    // !!!

    char *ptr = strtok(buf, " ");
    // assuming you have already sucessfully run file_get_ls()
    while(ptr != NULL)
    {
        printf("* %s\n", ptr);
        ptr = strtok(NULL, " ");
    }
}