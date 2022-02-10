#include <zlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "crc.h"

char crc_buf[MAX_CRC_BUF_SIZE];


uint32_t crc_generate(char *buf, uint32_t len)
{
    return crc32(0, (uint8_t*) buf, len);
}

uint32_t crc_generate_file(FILE *fp)
{
    uint32_t read = 0;
    uint32_t file_size = 0;
    uint32_t crc;

    // get file size
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // initiate variable
    crc = crc32(0L, Z_NULL, 0);

    while(file_size)
    {
        // read a chunk
        read = fread(
            crc_buf,     // location of buffer
            1,            // size of each element to read
            MAX_CRC_CHUNK_SIZE,   // number of elements to read
            fp            // input stream 
        );
        file_size -= read;

        crc = crc32(crc, (uint8_t*)crc_buf, read);

        memset(crc_buf, 0, sizeof(crc_buf));
    }

    return crc;


}