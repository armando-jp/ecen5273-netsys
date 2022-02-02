#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "packet.h"

// packet size = |PAYLOAD SIZE (4 bytes) | PAYLOAD (<=CHUNK_SIZE) | CRC32 (4 bytes)|
//                           4           +           500          +      4
uint32_t packet_generate(char *packet_buf, uint32_t packet_size, char *payload_buf, uint32_t payload_size, uint32_t crc32)
{
    // clear packet_buf
    memset(packet_buf, 0, packet_size);

    // copy size field into first 4 bytes of packet_buf
    memcpy(packet_buf, &payload_size, sizeof(uint32_t));

    // copy payload into packet_buf
    memcpy(packet_buf + 4, payload_buf, payload_size);

    // copy crc32 into last 4 bytes of packet_buf
    memcpy((packet_buf + packet_size + 4), &crc32, sizeof(uint32_t));

    return 4 + packet_size + 4;
}

void packet_print(char *packet_buf, uint32_t size)
{
    int i = 0;
    while(size)
    {
        printf("%02x ", packet_buf[i] & 0xFF);
        size--;
        i++;
    }
    printf("\n");
}

char *packet_get_buf(char *packet_buf)
{
    return packet_buf;
}

void packet_extract()
{

}

int packet_command(char *packet_buf, char *cmd, char *user_param)
{
    uint32_t size;

    // clear packet_buf
    memset(packet_buf, 0, sizeof(packet_buf));

    // copy size field into first 4 bytes of packet_buf
    printf("cmd, user_param = %s %s\n", cmd, user_param);

    memcpy(packet_buf, cmd, strlen(cmd));
    packet_buf[strlen(cmd)] = 0x20;
    memcpy(packet_buf + 1 + strlen(cmd), user_param, strlen(user_param));

    // snprintf(packet_buf, sizeof(packet_buf), "%s %s", cmd, user_param);
    printf("packet contents = %s\n", packet_buf);

    size = strlen(cmd) + 1 + strlen(user_param);
    return size;
}
