#include "cli.h"

const char *ws1=" \t";
const char *ws2=" \t\r\n";

static const
CLICMDS cli[NUM_COMMANDS] = {
    {"get",    "\tget <file_name>\r\n",    CMD_GET},
    {"put",    "\tput <file_name>\r\n",  CMD_PUT},
    {"delete", "\tdelete <file_name>\r\n", CMD_DELETE},
    {"ls",     "\tls\r\n",                 CMD_LS},
    {"exit",   "\texit\r\n",               CMD_EXIT},
};

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

/*
 * Find token in table.
 * Return command code, or 0 if not found.
 */
uint8_t match_command(char *token)
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
int16_t get_command(char *buf, char *param, uint16_t *param2)
{
    char *token = buf;
    int16_t rval = 0;
    uint8_t cmd;

    /* Find beginning of command... (skip leading spaces) */
    token = strtok(buf, ws2);
    cmd = match_command(token);

    /* Early exit if argless command  */
    if(cmd == 0 || cmd == CMD_EXIT || cmd == CMD_LS) 
    {
        return (cmd);
    }
        
    /* Switch on command */
    switch (cmd) {              

        // For CMD_GET we expect
        // "get <file_name>"
        case CMD_GET:
            token = strtok(NULL, ws2);
            strcpy(param, token);

            // if <file_name> not found, return 0.
            if (token == NULL) 
                cmd = 0;

            break;

        // For CMD_GPUT we expect
        // "put <file_name>"
        case CMD_PUT:
            token = strtok(NULL, ws2);
            strcpy(param, token);

            // if <file_name> not found, return 0.
            if (token == NULL) 
                cmd = 0;

            break;

        // For CMD_DELETE we expect
        // "delete <file_name>"
        case CMD_DELETE:
            token = strtok(NULL, ws2);
            strcpy(param, token);

            // if <file_name> not found, return 0.
            if (token == NULL) 
                cmd = 0;

            break;

    }
    return (cmd);
}