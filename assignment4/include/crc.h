#ifndef INC_CRC_
#define INC_CRC_

#define MAX_CRC_BUF_SIZE  (600)
#define MAX_CRC_CHUNK_SIZE (500)

uint32_t crc_generate(char *buf, uint32_t len);
uint32_t crc_generate_file(FILE *fp);

#endif /*INC_CRC_*/
