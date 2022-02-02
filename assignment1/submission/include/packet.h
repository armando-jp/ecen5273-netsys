#ifndef INC_PACKET_H_
#define INC_PACKET_H_


// function declerations
uint32_t packet_generate(char *packet_buf, uint32_t packet_size, char *payload_buf, uint32_t payload_size, uint32_t crc32);
void packet_print(char *packet_buf, uint32_t size);
char *packet_get_buf();
int packet_command(char *packet_buf, char *cmd, char *user_param);
#endif /*INC_PACKET_H_*/