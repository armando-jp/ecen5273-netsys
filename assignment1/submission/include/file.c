#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "file.h"
#include "crc.h"


FILE *fp;
char file_buf[MAX_FILE_BUF_SIZE];
uint32_t file_size;
uint32_t file_pointer = 0;

char *file_get_file_buf()
{
    return file_buf;
}

// opens a file for reading
int file_open(char* name)
{
    fp = fopen(name, "r");

    if(fp == NULL)
    {
        return -1;
    }
    return 0;
}

// returns file size in bytes
int file_get_size()
{
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

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

uint32_t file_get_fileptr_location()
{
    return file_pointer;
}

void file_close()
{
    fclose(fp);
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

FILE *file_get_fp()
{
    return fp;
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