#ifndef INC_CLI_
#define INC_CLI_


#include <stdint.h>
#include <string.h>

#define MAX_USER_INPUT (50)
#define MAX_USER_ARG   (50)

typedef struct _cli {
  char *text;
  char *help;
  uint16_t cmd;
} CLICMDS;

#define CMD_GET    1
#define CMD_PUT    2 
#define CMD_DELETE 3
#define CMD_LS     4
#define CMD_EXIT   5

#define NUM_COMMANDS 5

char *get_help(uint8_t cmd);
void cli_display_main_menu();
char *cli_get_user_response();
int16_t get_command(char *buf, char *param);

#endif /*INC_CLI_*/