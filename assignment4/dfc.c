/**
 * TODO: Implement GET command on client. Instructions have been written out.
 * After that, implement LS.
 * After that, implement 2 servers and splitting a file between them. Work your
 * way up to the 4 required. 
 */

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

char *dfs_name = NULL;

int main(int argc, char *argv[]) 
{
    // variable declerations
    int rv;
    FILE *fd_conf = NULL;
    char *user_resp = NULL;
    char user_param[MAX_USER_ARG];
    char file_path_buffer[MAX_FILE_NAME];
    fd_dfs_t fd;
    FILE *file;
    uint32_t crc32;

    /***************************************************************************
     * Verify number of arguments and process
     **************************************************************************/
    if(argc != 2)
    {
        msg_bad_args_client();
        return 0;
    }

    // save arguments to named variables
    char *dfc_filename = argv[1];
    printf("%s\r\n", dfc_filename);

    /***************************************************************************
     * Parse dfc.conf
     **************************************************************************/
    fd_conf = file_open(dfc_filename, 0);
    if(fd_conf == NULL)
    {
        printf("ERROR: %s does not exists\r\n", dfc_filename);
        return 0;
    }

    conf_results_t conf = conf_parsing_get_config_dfc(fd_conf);
    conf_parsing_print_struct(conf);


    /***************************************************************************
     * Connect to servers
     **************************************************************************/
    // connect to DFS1
    fd.dfs1 = sock_connect_to_host(conf.dfs1_addr, conf.dfs1_port);
    if(fd.dfs1 == -1)
    {
        printf("ERROR: Failed to connect to dfs1.\r\n");
        // return 0;
    }
    // connect to DFS2
    fd.dfs2 = sock_connect_to_host(conf.dfs2_addr, conf.dfs2_port);
    if(fd.dfs2 == -1)
    {
        printf("ERROR: Failed to connect to dfs2.\r\n");
        // return 0;
    }
    // connect to DFS3
    fd.dfs3 = sock_connect_to_host(conf.dfs3_addr, conf.dfs3_port);
    if(fd.dfs3 == -1)
    {
        printf("ERROR: Failed to connect to dfs3.\r\n");
        // return 0;
    }
    // connect to DFS4
    fd.dfs4 = sock_connect_to_host(conf.dfs4_addr, conf.dfs4_port);
    if(fd.dfs4 == -1)
    {
        printf("ERROR: Failed to connect to dfs4.\r\n");
        // return 0;
    }
    
    /***************************************************************************
     * Super loop
     **************************************************************************/
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
                // printf("GET NOT IMPLEMENTED YET\r\n");
                if(file_exists(user_param) == 0)
                {
                    // 1. Get and merge file pieces
                    sm_get(user_param, conf, fd);
                    sm_merge(user_param, conf);

                    // 2. Delete the chunks locally
                    memset(file_path_buffer, 0, MAX_FILE_NAME);
                    sprintf(file_path_buffer, ".%s.1", user_param);
                    file_delete(file_path_buffer);

                    memset(file_path_buffer, 0, MAX_FILE_NAME);
                    sprintf(file_path_buffer, ".%s.2", user_param);
                    file_delete(file_path_buffer);

                    memset(file_path_buffer, 0, MAX_FILE_NAME);
                    sprintf(file_path_buffer, ".%s.3", user_param);
                    file_delete(file_path_buffer);

                    memset(file_path_buffer, 0, MAX_FILE_NAME);
                    sprintf(file_path_buffer, ".%s.4", user_param);
                    file_delete(file_path_buffer);
                }
                else
                {
                    printf("Clinet: %s already exists on client.\r\n", user_param);
                }

            break;

            case CMD_PUT:
                // 1. Create 4 files
                printf("Client: Splitting files\r\n");
                int res = file_split(user_param);
                if(res == -1)
                {
                    printf("Failed to split %s\r\n", user_param);
                }
                
                // 2. Calculate which server gets which piece
                printf("Client: Calculating file distribution\r\n");
                file = file_open(user_param, 0);
                if(file == NULL)
                {
                    printf("Failed to open %s\r\n", user_param);
                }
                crc32 = crc_generate_file(file);
                file_close(file);
                printf("crc32 for %s is %u\r\n", user_param, crc32);
                printf("%u %% 4 = %d\r\n", crc32, crc32%4);

                // 3. Send file chunks to servers
                printf("Client: Distributing file chunks\r\n");
                switch(crc32 % 4)
                {
                    case 0:
                        // send .file_name.1 & .file_name.2 to DFS1
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        // send .file_name.2 & .file_name.3 to DFS2
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        // send .file_name.3 & .file_name.4 to DFS3
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        // send .file_name.4 & .file_name.1 to DFS4
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);
                    break;

                    case 1:
                        // send .file_name.4 & .file_name.1 to DFS1
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        // send .file_name.1 & .file_name.2 to DFS2
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        // send .file_name.2 & .file_name.2 to DFS3
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        // send .file_name.3 & .file_name.4 to DFS4
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);
                    break;

                    case 2:
                        // send .file_name.3 & .file_name.4 to DFS1
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        // send .file_name.4 & .file_name.1 to DFS2
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        // send .file_name.1 & .file_name.2 to DFS3
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        // send .file_name.2 & .file_name.3 to DFS4
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);
                    break;

                    case 3:
                        // send .file_name.2 & .file_name.3 to DFS1
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs1, 0);

                        // send .file_name.3 & .file_name.4 to DFS2
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.3", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs2, 0);

                        // send .file_name.4 & .file_name.1 to DFS3
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.4", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs3, 0);

                        // send .file_name.1 & .file_name.2 to DFS4
                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.1", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);

                        memset(file_path_buffer, 0, MAX_FILE_NAME);
                        sprintf(file_path_buffer, ".%s.2", user_param);
                        sm_send(file_path_buffer, conf, fd.dfs4, 0);
                    break;

                    default:
                        printf("ERROR: Not supposed to be here\r\n");
                    break;
                }

                // 4. Delete the chunks locally
                memset(file_path_buffer, 0, MAX_FILE_NAME);
                sprintf(file_path_buffer, ".%s.1", user_param);
                file_delete(file_path_buffer);

                memset(file_path_buffer, 0, MAX_FILE_NAME);
                sprintf(file_path_buffer, ".%s.2", user_param);
                file_delete(file_path_buffer);

                memset(file_path_buffer, 0, MAX_FILE_NAME);
                sprintf(file_path_buffer, ".%s.3", user_param);
                file_delete(file_path_buffer);

                memset(file_path_buffer, 0, MAX_FILE_NAME);
                sprintf(file_path_buffer, ".%s.4", user_param);
                file_delete(file_path_buffer);

            break;

            case CMD_LS:
                // printf("LS NOT IMPLEMENTED YET\r\n");
                sm_client_ls(fd, conf, user_param);
                // sm_client_ls();
            break;

            case CMD_EXIT:
                return 0;
            break;
        }

    }

    return 0;
}