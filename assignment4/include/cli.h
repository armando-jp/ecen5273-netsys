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
#define CMD_LS     3
#define CMD_EXIT   4

#define NUM_COMMANDS 4

/*******************************************************************************
 * User Input Processing Functions
*******************************************************************************/
void cli_generate_filtered_usr_cmd(char *cmd, char *param);
uint32_t cli_get_filtered_usr_cmd_size();
char *cli_get_filtered_usr_cmd();
int16_t get_command(char *buf, char *param);

/*******************************************************************************
 * Get command help Functions
*******************************************************************************/
char *get_help(uint8_t cmd);

/*******************************************************************************
 * Display Functions
*******************************************************************************/
void cli_display_main_menu();
char *cli_get_user_response();

/*******************************************************************************
 * Getter/Setter Functions
*******************************************************************************/
char *cli_get_user_input_buf();
char *cli_get_cmd_filtered_buf();
char *cli_get_user_param_buf();

#endif /*INC_CLI_*/