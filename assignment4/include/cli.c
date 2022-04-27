#include "cli.h"
#include <stdio.h>

const char *ws1=" \t";
const char *ws2=" \t\r\n";

static char user_input[MAX_USER_INPUT];
static char cmd_filtered[MAX_USER_INPUT];
static char user_param[MAX_USER_INPUT];

static const
CLICMDS cli[NUM_COMMANDS] = {
    {"get",    "\tget <file_name>\r\n",    CMD_GET},
    {"put",    "\tput <file_name>\r\n",    CMD_PUT},
    {"ls",     "\tls\r\n",                 CMD_LS},
    {"exit",   "\texit\r\n",               CMD_EXIT},
};

/*******************************************************************************
 * User Input Processing Functions
*******************************************************************************/
// generates a filtered string of the users command
void cli_generate_filtered_usr_cmd(char *cmd, char *param)
{
    char *last;
    const char ch = '/';

    memset(cmd_filtered, 0, MAX_USER_INPUT);

    // parse file name if contains '/'
    last = strrchr(param, ch);

    // generate filtered cmd string
    if(last != NULL)
    {
        sprintf(cmd_filtered, "%s %s", cmd, last+1);
    }
    else
    {
        sprintf(cmd_filtered, "%s %s", cmd, param);
    }

}

uint32_t cli_get_filtered_usr_cmd_size()
{
    return strlen(cmd_filtered);
}

char *cli_get_filtered_usr_cmd()
{
    return cmd_filtered;
}

/*
 * Find token in table.
 * Return command code, or 0 if not found.
 */
static uint8_t match_command(char *token)
{
    uint8_t i = NUM_COMMANDS;
    for (i = 0; i < NUM_COMMANDS; i++)
       if (strcmp(token, cli[i].text) == 0)
           return cli[i].cmd;
    return (0);
}

/*
 * Scan buffer for command.
 * Return CMD value, or 0 if no command.
 * If command has a arg, return in *param.
 */
int16_t get_command(char *buf, char *param)
{
    char *token = buf;
    uint8_t cmd;

    /* Find beginning of command... (skip leading spaces) */
    token = strtok(buf, ws2);
    cmd = match_command(token);

    /* Early exit if argless command  */
    if(cmd == 0 || cmd == CMD_EXIT) 
    {
        return (cmd);
    }
        
    /* Switch on command */
    switch (cmd) {       

        // For CMD_GET we expect
        // "get <file_name>"
        case CMD_GET:
            token = strtok(NULL, ws2);

            // if <file_name> not found, return 0.
            if (!token) 
            {
                cmd = 0;
            }
            else
            {
                strcpy(param, token);
            }
        break;
                


        // For CMD_PUT we expect
        // "put <file_name>"
        case CMD_PUT:
            token = strtok(NULL, ws2);
            strcpy(param, token);

            // if <file_name> not found, return 0.
            if (token == NULL) 
            {
                cmd = 0;
            }

        break;

        // For CMD_LS we expect
        // "ls <file_name>"
        case CMD_LS:
            token = strtok(NULL, ws2);
            strcpy(param, token);

            // if <file_name> not found, return 0.
            if (token == NULL) 
            {
                cmd = 0;
            }

        break;

    }
    strcpy(user_param, param);
    return (cmd);
}

/*******************************************************************************
 * Get command help Functions
*******************************************************************************/
// returns the help string specified by the 'cmd' arg
char *get_help(uint8_t cmd)
{
    uint8_t i = 0;
    char *rval = NULL;

    for (i = 0; i < NUM_COMMANDS; i++) {
       if (cmd == cli[i].cmd) {
           rval = cli[i].help;
           break;
       }
    }
    return (rval);
}

/*******************************************************************************
 * Display Functions
*******************************************************************************/
void cli_display_main_menu()
{
    printf("Main Menu\n");
    int i;
    for(i = 1; i <= NUM_COMMANDS; i++)
    {
        printf("%d. %s", i, get_help(i));
    }
}

char *cli_get_user_response()
{
    printf("> ");
    fgets(user_input, MAX_USER_INPUT, stdin);
    return user_input;
}

/*******************************************************************************
 * Getter/Setter Functions
*******************************************************************************/
char *cli_get_user_input_buf()
{
    return user_input;
}

char *cli_get_cmd_filtered_buf()
{
    return cmd_filtered;
}

char *cli_get_user_param_buf()
{
    return user_param;
}