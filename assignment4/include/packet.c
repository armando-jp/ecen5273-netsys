#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "packet.h"

/*******************************************************************************
* Functions for generating and parsing packets
*******************************************************************************/
Packet packet_get_default()
{
    Packet pkt;

    memset(&pkt, 0, sizeof(pkt));

    pkt.user_name[0]   = '\0';
    pkt.password[0]    = '\0';
    pkt.cmd_header     = CMD_NULL_PKT;
    pkt.crc32_header   = 0;
    pkt.file_name[0]      = '\0';
    pkt.payload_header = 0;
    pkt.payload[0]     = 0;

    return pkt;
}

uint32_t packet_convert_to_buffer(Packet pkt, char *buffer)
{
    uint32_t offset = 0;

    // Get user_name
    memcpy(buffer+offset, &pkt.user_name, USER_NAME_SIZE);
    offset += USER_NAME_SIZE;

    // Get password
    memcpy(buffer+offset, &pkt.password, PASSWORD_SIZE);
    offset += PASSWORD_SIZE;  

    // Get cmd_header
    memcpy(buffer+offset, &pkt.cmd_header, CMD_CODE);
    offset += CMD_CODE;  

    // Get crc32_header
    memcpy(buffer+offset, &pkt.crc32_header, CRC32_HEADER);
    offset += CRC32_HEADER;  

    // Get file_name
    memcpy(buffer+offset, pkt.file_name, FILE_NAME_SIZE);
    offset += FILE_NAME_SIZE;  

    // Get payload_header
    memcpy(buffer+offset, &pkt.payload_header, PAYLOAD_HEADER);
    offset += PAYLOAD_HEADER; 

    // Get payload
    if(pkt.payload_header > 0 && pkt.payload_header <= PAYLOAD_CHUNK_SIZE)
    {
        memcpy(buffer+offset, pkt.payload, pkt.payload_header);
        offset += pkt.payload_header;   
    }
    else if(pkt.payload_header > PAYLOAD_CHUNK_SIZE)
    {
        memcpy(buffer+offset, pkt.payload, PAYLOAD_CHUNK_SIZE);
        offset += PAYLOAD_CHUNK_SIZE;     
    }

    return offset;
}
/*******************************************************************************
* Functions for parsing packets from buffer
*******************************************************************************/
Packet packet_parse_packet(char *buffer, uint32_t buffer_size)
{
    uint32_t offset = 0;
    Packet pkt = packet_get_default();

    if(buffer_size > PACKET_SIZE)
    {
        return pkt;
    }

    // Get user_name
    memcpy(pkt.user_name, buffer+offset, USER_NAME_SIZE);
    offset += USER_NAME_SIZE;

    // Get password
    memcpy(pkt.password, buffer+offset, PASSWORD_SIZE);
    offset += PASSWORD_SIZE;  

    // Get cmd_header
    memcpy(&pkt.cmd_header, buffer+offset, CMD_CODE);
    offset += CMD_CODE;  

    // Get crc32_header
    memcpy(&pkt.crc32_header, buffer+offset, CRC32_HEADER);
    offset += CRC32_HEADER;  

    // Get file_name 
    memcpy(pkt.file_name, buffer+offset, FILE_NAME_SIZE);
    offset += FILE_NAME_SIZE;  

    // Get payload_header
    memcpy(&pkt.payload_header, buffer+offset, PAYLOAD_HEADER);
    offset += PAYLOAD_HEADER; 

    // Get payload
    if(pkt.payload_header > 0 && pkt.payload_header <= PAYLOAD_CHUNK_SIZE)
    {
        memcpy(pkt.payload, buffer+offset, pkt.payload_header);
        offset += pkt.payload_header;   
    }
    else if(pkt.payload_header > PAYLOAD_CHUNK_SIZE)
    {
        memcpy(pkt.payload, buffer+offset, PAYLOAD_CHUNK_SIZE);
        offset += PAYLOAD_CHUNK_SIZE; 
    }

    return pkt;
}

/*******************************************************************************
* Utility functions
*******************************************************************************/
bool packet_is_default(Packet pkt)
{
    if(pkt.user_name[0] != '\0')
    {
        return false;
    }

    if(pkt.password[0] != '\0')
    {
        return false;
    }

    if(pkt.cmd_header != CMD_NULL_PKT)
    {
        return false;
    }

    if(pkt.crc32_header != 0)
    {
        return false;
    }

    if(pkt.file_name[0] != '\0')
    {
        return false;
    }

    if(pkt.payload_header != 0)
    {
        return false;
    }

    if(pkt.payload[0] != '\0')
    {
        return false;
    }

    return true;
}

void packet_print(Packet pkt)
{
    printf("\tPRINTING PKT CONTENTS\r\n");
    printf("\tuser_name: %s\r\n", pkt.user_name);
    printf("\tpassword: %s\r\n", pkt.password);
    printf("\tcmd_header: %d\r\n", pkt.cmd_header);
    printf("\tcrc32_header: %u\r\n", pkt.crc32_header);
    printf("\tfile_name: %s\r\n", pkt.file_name);
    printf("\tpayload_header: %d\r\n", pkt.payload_header);   
}