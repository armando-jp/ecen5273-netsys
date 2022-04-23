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

void file_set_fp(FILE *fp, int location)
{
    fseek(fp, location, SEEK_SET);
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

FILE *file_open_create(char *name)
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



/*******************************************************************************
 * Split file
 ******************************************************************************/
int file_split(char *file_name)
{
    FILE *file;
    FILE *new_file;
    int file_size;
    int chunk1_size;
    int chunk2_size;
    int chunk3_size;
    int chunk4_size;
    int bytes;
    char new_file_name[30];
    char buffer[512];

    // Check if file exists
    file = file_open(file_name, 0);
    if(file == NULL)
    {
        printf("%s does not exist\r\n", file_name);
        return -1;
    }

    // Get file size
    file_size =  file_get_size(file);
    printf("%s size: %d\r\n", file_name, file_size);

    /***************************************************************************
     * Write `.1.file_name.2`
     **************************************************************************/
    // Calculate each file size
    chunk1_size = file_size / 4;
    chunk2_size = file_size / 4;
    chunk3_size = file_size / 4;
    chunk4_size = (file_size / 4) + (file_size - ((int)(file_size/4) * 4));
    printf("File sizes: 1:%d 2:%d 3:%d 4:%d\r\n", chunk1_size, chunk2_size, chunk3_size, chunk4_size);
    // return -1;

    // Create new file name
    memset(new_file_name, 0, 30);
    sprintf(new_file_name, ".1.%s.2", file_name);

    // open the file 
    new_file = file_open_create(new_file_name);
    if(new_file == NULL)
    {
        printf("unable to create %s\r\n", new_file_name);
    }

    // write the chunk sizes into the file
    bytes = sprintf(buffer, "%d\r\n%d\r\n", chunk1_size, chunk2_size);
    file_write(buffer, new_file, bytes);

    // Write `.1.file_name.2` CHUNK 1
    while(chunk1_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk1_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk1_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk1_size -= bytes;
    }

    // Write `.1.file_name.2` CHUNK 2
    while(chunk2_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk2_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk2_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk2_size -= bytes;
    }

    // Close file
    file_close(new_file);

    /***************************************************************************
     * Write `.2.file_name.3`
     **************************************************************************/
    // Calculate each file size
    chunk1_size = file_size / 4;
    chunk2_size = file_size / 4;
    chunk3_size = file_size / 4;
    chunk4_size = (file_size / 4) + (file_size - ((int)(file_size/4) * 4));

    // Create new file name
    memset(new_file_name, 0, 30);
    sprintf(new_file_name, ".2.%s.3", file_name);

    // open the file 
    new_file = file_open_create(new_file_name);
    if(new_file == NULL)
    {
        printf("unable to create %s\r\n", new_file_name);
    }

    // write the chunk sizes into the file
    bytes = sprintf(buffer, "%d\r\n%d\r\n", chunk2_size, chunk3_size);
    file_write(buffer, new_file, bytes);

    // Write `.2.file_name.3` CHUNK 2
    file_set_fp(file, chunk1_size);
    while(chunk2_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk2_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk2_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk2_size -= bytes;
    }

    // Write `.2.file_name.3` CHUNK 3
    while(chunk3_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk3_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk3_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk3_size -= bytes;
    }

    // Close file
    file_close(new_file);

    /***************************************************************************
     * Write `.3.file_name.4`
     **************************************************************************/
    // Calculate each file size
    chunk1_size = file_size / 4;
    chunk2_size = file_size / 4;
    chunk3_size = file_size / 4;
    chunk4_size = (file_size / 4) + (file_size - ((int)(file_size/4) * 4));

    // Create new file name
    memset(new_file_name, 0, 30);
    sprintf(new_file_name, ".3.%s.4", file_name);

    // open the file 
    new_file = file_open_create(new_file_name);
    if(new_file == NULL)
    {
        printf("unable to create %s\r\n", new_file_name);
    }

    // write the chunk sizes into the file
    bytes = sprintf(buffer, "%d\r\n%d\r\n", chunk3_size, chunk4_size);
    file_write(buffer, new_file, bytes);

    // Write `.3.file_name.4` CHUNK 3
    file_set_fp(file, chunk1_size+chunk2_size);
    while(chunk3_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk3_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk3_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk3_size -= bytes;
    }

    // Write `.3.file_name.4` CHUNK 4
    while(chunk4_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk4_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk4_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk4_size -= bytes;
    }

    // Close file
    file_close(new_file);

    /***************************************************************************
     * Write `.4.file_name.1`
     **************************************************************************/
    // Calculate each file size
    chunk1_size = file_size / 4;
    chunk2_size = file_size / 4;
    chunk3_size = file_size / 4;
    chunk4_size = (file_size / 4) + (file_size - ((int)(file_size/4) * 4));

    // Create new file name
    memset(new_file_name, 0, 30);
    sprintf(new_file_name, ".4.%s.1", file_name);

    // open the file 
    new_file = file_open_create(new_file_name);
    if(new_file == NULL)
    {
        printf("unable to create %s\r\n", new_file_name);
    }

    // write the chunk sizes into the file
    bytes = sprintf(buffer, "%d\r\n%d\r\n", chunk4_size, chunk1_size);
    file_write(buffer, new_file, bytes);

    // Write `.4.file_name.1` CHUNK 4
    file_set_fp(file, chunk1_size+chunk2_size+chunk3_size);
    while(chunk4_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk4_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk4_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk4_size -= bytes;
    }

    // Write `.4.file_name.1` CHUNK 1
    file_set_fp(file, 0);
    while(chunk1_size)
    {
        // read a chunk
        memset(buffer, 0, 512);
        if(chunk1_size > 512)
        {
            bytes = file_read(buffer, file, 512);
        }
        else
        {
            bytes = file_read(buffer, file, chunk1_size);
        }

        // write a chunk
        file_write(buffer, new_file, bytes);

        chunk1_size -= bytes;
    }

    // Close file
    file_close(new_file);
    file_close(file);

    return 0;
}