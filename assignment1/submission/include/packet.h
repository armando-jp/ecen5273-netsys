#ifndef INC_PACKET_H_
#define INC_PACKET_H_


#define SEQ_NUM_FIELD      (4)
#define PAYLOAD_SIZE_FIELD (4)
#define CHUNK_SIZE         (500)
#define CRC32_FIELD        (4)

#define MAX_PAYLOAD (4 + CHUNK_SIZE + 4)

typedef struct Packet {
    uint32_t payload_size;
    char payload[CHUNK_SIZE];
    uint32_t crc32;
    uint32_t total_size;
} Packet;


/*******************************************************************************
* Functions for generating and parsing packets
*******************************************************************************/
uint32_t packet_generate();
void packet_parse(char *buf);

/*******************************************************************************
* Functions for getting and writing to packet struct
*******************************************************************************/
char *packet_get_payload();
uint32_t packet_get_payload_size();
uint32_t packet_get_crc32();
uint32_t packet_get_total_size();
void packet_write_payload(char *buf, uint32_t size);
void packet_write_payload_size(uint32_t size);
void packet_write_crc32(uint32_t crc32);
void packet_write_total_size(uint32_t size);
char *packet_get_buf();

/*******************************************************************************
* Utility functions
*******************************************************************************/
void packet_print(char *buf, uint32_t size);
void packet_print_struct();
uint32_t packet_get_chunk_size();

#endif /*INC_PACKET_H_*/