#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "./include/msg.h"
#include "./include/cli.h"
#include "./include/utils.h"
#include "./include/socket.h"
#include "./include/timer.h"
#include "./include/file.h"
#include "./include/crc.h"
#include "./include/packet.h"
#include "./include/state_machine.h"
#include "./include/conf_parsing.h"

#define DEBUG         (0)


int main(int argc, char *argv[]) 
{
    // variable declerations
    int rv;
    FILE *fd_conf = NULL;
    char *user_resp = NULL;
    char user_param[MAX_USER_ARG];

    int fd_dfs1;
    int fd_dfs2;
    int fd_dfs3;
    int fd_dfs4;

    // verify the correct number of arguments
    if(argc != 2)
    {
        msg_bad_args_client();
        return 0;
    }

    // save arguments to named variables
    char *dfc_filename = argv[1];
    printf("%s\r\n", dfc_filename);

    // check dfc.conf exists
    fd_conf = file_open(dfc_filename, 0);
    if(fd_conf == NULL)
    {
        printf("ERROR: %s does not exists\r\n", dfc_filename);
        return 0;
    }

    // parse dfc_conf
    conf_results_t conf = conf_parsing_get_config_dfc(fd_conf);
    conf_parsing_print_struct(conf);

    // attempt to connnect to servers
    // connect to DFS1
    fd_dfs1 = sock_connect_to_host(conf.dfs1_addr, conf.dfs1_port);
    if(fd_dfs1 == -1)
    {
        printf("ERROR: Failed to connect to dfs1.\r\n");
        return 0;
    }
    // connect to DFS2
    // connect to DFS3
    // connect to DFS4
    
    // enter super loop
    while(1)
    {
        // display main menu and process user response
        cli_display_main_menu();
        user_resp = cli_get_user_response();
        memset(user_param, 0, MAX_USER_ARG);
        rv = get_command(user_resp, user_param);
        
        if(!rv)
        {
            msg_bad_command();
            continue;
        }

        switch(rv)
        {
            case CMD_GET:
                // sm_client_get();
            break;

            case CMD_PUT:
                // sm_client_put();
            break;

            case CMD_LS:
                // sm_client_ls();
            break;

            case CMD_EXIT:
                return 0;
            break;
        }

    }

    return 0;
}