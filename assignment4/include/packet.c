#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "packet.h"

/*
 * The way to use this library is to first load the packet struct with:
 *      * frame number (uint32_t)
 *      * payload size (uint32_t)
 *      * payload      (char array[<=MAX_PAYLOAD])
 *      * crc32        (uint32_t)
 * Using the packet_write_*() commands.
 * 
 * Then, after loading the struct, use packet_generate() to populate the char 
 * array which will be transmitted via UDP.
 * 
 * On the receiving device, the usage is to use packet_parse() to populate
 * the packet struct. Packet struct fields can then be accessed using the 
 * packet_get_*() methods.
 * 
 * Use https://crccalc.com/ to verify generated crc32 values.
 */

// Struct for holding parsed packet contents
static Packet packet = {0};
// Char array for holding unparsed packet contents
static char packet_buf[MAX_PAYLOAD];

/*******************************************************************************
* Functions for generating and parsing packets
*******************************************************************************/
uint32_t packet_generate()
{
    // clear packet_buf
    memset(packet_buf, 0, MAX_PAYLOAD);

    // copy sequence number field into first 4 bytes of packet_buf
    memcpy(packet_buf, &packet.sequence_number, sizeof(packet.sequence_number));

    // copy size field into second 4 bytes of packet_buf
    memcpy(packet_buf + sizeof(packet.sequence_number), &packet.payload_size, sizeof(packet.payload_size));

    // copy payload into packet_buf
    memcpy(packet_buf + sizeof(packet.sequence_number) + sizeof(packet.payload_size), packet.payload, packet.payload_size);

    // copy crc32 into last 4 bytes of packet_buf
    memcpy((packet_buf + sizeof(packet.sequence_number) + packet.payload_size + sizeof(packet.payload_size)), &packet.crc32, sizeof(packet.crc32));

    //             4 bytes                +                 4 bytes     + <=500 bytes  +    4 bytes
    return sizeof(packet.sequence_number) + sizeof(packet.payload_size) + packet.payload_size + sizeof(packet.crc32);
}


// Used by receiving device. Used to parse a buffer into the packet struct.
void packet_parse(char *buf)
{
    // copy sequence number field
    memcpy(&packet.sequence_number, buf, sizeof(packet.sequence_number));
    // copy size field
    memcpy(&packet.payload_size, buf + sizeof(packet.sequence_number), sizeof(packet.payload_size));
    // copy payload
    memcpy(packet.payload, sizeof(packet.sequence_number) + sizeof(packet.payload_size) + buf, packet.payload_size);
    // copy crc32 field 
    memcpy(&packet.crc32, sizeof(packet.sequence_number) + sizeof(packet.payload_size) + buf + packet.payload_size, sizeof(packet.crc32));

}


/*******************************************************************************
* Functions for getting and writing to packet struct
*******************************************************************************/
char *packet_get_payload()
{
    return packet.payload;
}

uint32_t packet_get_payload_size()
{
    return packet.payload_size;
}

uint32_t packet_get_crc32()
{
    return packet.crc32;
}

uint32_t packet_get_sequence_number()
{
    return packet.sequence_number;
}

void packet_write_payload(char *buf, uint32_t size)
{
    memset(packet.payload, 0, CHUNK_SIZE);
    memcpy(packet.payload, buf, size);
}

void packet_write_payload_size(uint32_t size)
{
    packet.payload_size = size;
}

void packet_write_crc32(uint32_t crc32)
{
    packet.crc32 = crc32;
}

void packet_write_sequence_number(uint32_t size)
{
    packet.sequence_number = size;
}

char *packet_get_buf()
{
    return packet_buf;
}

uint32_t packet_get_packet_size_for_crc()
{
    return sizeof(packet.sequence_number) + sizeof(packet.payload_size) + packet.payload_size;
}

uint32_t packet_get_total_size()
{
    return sizeof(packet.sequence_number) + sizeof(packet.payload_size) + packet.payload_size + sizeof(packet.crc32);
}

Packet *packet_get_struct()
{
    return &packet;
}
/*******************************************************************************
* Utility functions
*******************************************************************************/
void packet_print(char *buf, uint32_t size)
{
    int i = 0;
    while(size)
    {
        printf("%02x ", buf[i] & 0xFF);
        size--;
        i++;
    }
    printf("\n");
}

void packet_print_struct()
{
    printf("PACKET SEQUENCE NUMBER: %u\n", packet.sequence_number);
    printf("SIZE OF PAYLOAD: %u\n", packet.payload_size);
    printf("PAYLOAD CONTENTS:\n");
    packet_print(packet.payload, packet.payload_size);
    printf("PACKET CRC32: %u\n", packet.crc32);
}

uint32_t packet_get_chunk_size()
{
    return CHUNK_SIZE;
}