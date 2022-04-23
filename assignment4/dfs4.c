#include <stdio.h>
#include <dirent.h>

#include "./include/socket.h"
#include "./include/file.h"
#include "./include/conf_parsing.h"
#include "./include/state_machine.h"

char *dfs_name = "DFS4";
char *dfs_dir = "./DFS4";
char *dfs_conf_name = "dfs.conf";


int main(int argc, char *argv[])
{
    int sockfd;
    FILE *fd_conf;
    DIR *dir = NULL;

    // verify the correct number of arguments
    if(argc != 2)
    {
        printf("%s: Invalid args\r\n", dfs_name);
        return 0;
    }

    // Get port argument
    char *dfs_port = argv[1];

    // Open or create dfs# directory.
    dir = file_open_dir(dfs_dir);
    if(dir == NULL)
    {
        printf("Failed to open %s\r\n", dfs_dir);
        return 0;
    }

    // Attempt to open dfs.conf
    fd_conf = file_open(dfs_conf_name, 0);
    if(fd_conf == NULL)
    {
        printf("ERROR: %s does not exists\r\n", dfs_conf_name);
        return 0;
    }

    // parse dfs_conf
    conf_results_dfs_t conf = conf_parsing_get_config_dfs(fd_conf);
    conf_parsing_print_struct_dfs(conf);

    // Bind to port
    printf("%s: Binding to port %s\r\n", dfs_name, dfs_port);
    sockfd = sock_bind_to_port(dfs_port);
    if(sockfd == -1)
    {
        printf("%s: Failed to bind to port %s.\r\n", dfs_name, dfs_port);
        return 0;
    }

    // create thread
    sm_server(sockfd);

    // shouldn't get here
    sock_close(sockfd);

    return 0;
}