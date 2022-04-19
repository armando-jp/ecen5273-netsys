#ifndef INC_PACKET_H_
#define INC_PACKET_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * PACKET DESCRIPTION
 * 
 * 11 - user_name
 * 21 - password
 * 1  - cmd_code
 * 4  - crc32_header
 * 4  - payload_header
 * #  - payload
 ******************************************************************************/

#define USER_NAME_SIZE            (11)
#define PASSWORD_SIZE             (21)
#define CMD_CODE                  (1)
#define CRC32_HEADER              (4)
#define FILE_NAME_SIZE            (21)
#define PAYLOAD_HEADER            (4)
#define PAYLOAD_CHUNK_SIZE        (512)
#define PACKET_SIZE_MINUS_PAYLOAD (USER_NAME_SIZE + PASSWORD_SIZE + CMD_CODE + CRC32_HEADER + FILE_NAME_SIZE + PAYLOAD_HEADER)
#define PACKET_SIZE               (PACKET_SIZE_MINUS_PAYLOAD + PAYLOAD_CHUNK_SIZE)

#define CMD_NULL_PKT (0)
#define CMD_GET_PKT  (1)
#define CMD_PUT_PKT  (2)
#define CMD_LS_PKT   (3)

typedef struct Packet {
    char user_name[USER_NAME_SIZE];
    char password[PASSWORD_SIZE];
    uint8_t cmd_header;
    uint32_t crc32_header;
    char file_name[FILE_NAME_SIZE];
    uint32_t payload_header;
    char payload[PAYLOAD_CHUNK_SIZE];
} Packet;

/*******************************************************************************
* Functions for generating and parsing packets
*******************************************************************************/
Packet packet_get_default();
uint32_t packet_convert_to_buffer(Packet pkt, char *buffer);

/*******************************************************************************
* Functions for parsing packets from buffer
*******************************************************************************/
Packet packet_parse_packet(char *buffer, uint32_t buffer_size);

/*******************************************************************************
* Utility functions
*******************************************************************************/
bool packet_is_default(Packet pkt);
void packet_print(Packet pkt);

#endif /*INC_PACKET_H_*/