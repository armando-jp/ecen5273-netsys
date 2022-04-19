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
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "file.h"
#include "crc.h"

/*******************************************************************************
 * Utility functions
*******************************************************************************/
// returns file size in bytes
int file_get_size(FILE *fp)
{
    int file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

/*******************************************************************************
 * Functions for OPENING and CLOSING files
*******************************************************************************/
// opens a file for reading
FILE *file_open(char* name, bool mode)
{
    FILE *fp;

    if(access(name, F_OK) != 0) 
    {
        return NULL;
    } 
    
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

FILE *file_open_create(char *name, bool mode)
{
    FILE *fp;
    
    fp = fopen(name, "w+");
    return fp;   
}

void file_close(FILE *fp)
{
    fclose(fp);
}


/*******************************************************************************
 * Functions for READING and WRITING files
*******************************************************************************/
uint32_t file_read(char *buffer, FILE *fp, uint32_t bytes)
{
    uint32_t total_read = 0;
    uint32_t offset = 0;

    while(bytes)
    {
        total_read = fread(
            buffer+offset,       // location of buffer
            1,            // size of each element to read
            bytes,        // number of elements to read
            fp            // input stream 
        );
        bytes-=total_read;
        offset+=total_read;
    }

    return total_read;
}

uint32_t file_write(char* buffer, FILE *fp, uint32_t bytes)
{
    uint32_t total_write = 0;

    total_write = fwrite(
        buffer,       // location of buffer
        1,            // size of each element to write
        bytes,        // number of elements to write
        fp            // output stream 
    );
    return total_write;
}

/*******************************************************************************
 * Functions for DELETING files
*******************************************************************************/
int file_delete(char * file_name)
{
    return remove(file_name);
}

// /*******************************************************************************
//  * Functions for getting directory contents (LS)
// *******************************************************************************/
// static int file_select(const struct direct *entry)
// {
//     if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
//     {
//         return (0);
//     }
//     else
//     {
//         return (1);
//     }
// }

// int file_get_ls(char* buffer, uint32_t buffer_size)
// {
//     int count;
//     int i;
//     struct direct **files;

//     memset(buffer, 0, buffer_size);

//     if(!getcwd(buffer, sizeof(buffer)))
//     {
//         printf("Error getting pathname\n");
//         return -1;
//     }
//     // printf("Pathname obtained: %s\n", buffer);

//     count = scandir(buffer, &files, file_select, alphasort);

//     // If no files found,make a non-selectable menu item
//     if(count <= 0)
//     {
//         printf("No files in this directory\n");
//         return -1;
//     }

//     // save file names info buffer
//     memset(buffer, 0, buffer_size);
//     printf("Number of files = %d\n", count);
//     for (i=0; i < count; ++i)
//     {
//         strcat(buffer, files[i]->d_name);
//         strncat(buffer, " \n", 1);

//         //printf("%s  ", files[i]->d_name);
//     }
//     // printf("\n");

//     return strlen(buffer);
// }

// void file_print_ls_buf(char *buf)
// {
//     // !!!
//     //THIS FUNCTION IS DESTRUCTIVE. buffer[] WILL BE PERMANENTLY MODIFIED.
//     // !!!

//     char *ptr = strtok(buf, " ");
//     // assuming you have already sucessfully run file_get_ls()
//     while(ptr != NULL)
//     {
//         printf("* %s\n", ptr);
//         ptr = strtok(NULL, " ");
//     }
// }


DIR *file_open_dir(const char *dir_name)
{
    DIR* dir = opendir(dir_name);
    if (dir) {
        /* Directory exists. */
    } else if (ENOENT == errno) {
        dir = NULL;
        /* Directory does not exist. */
        int ret = mkdir(dir_name, S_IRWXU);
        if(ret != -1)
        {
            dir = opendir(dir_name);
        }
    } else {
        dir = NULL;
        /* opendir() failed for some other reason. */
    }
    return dir;
}
