#include "conf_parsing.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Initialization functions
 ******************************************************************************/
conf_results_t conf_parsing_default_struct()
{
    conf_results_t res;

    res.dfs1_addr[0] = '\0';
    res.dfs2_addr[0] = '\0';
    res.dfs3_addr[0] = '\0';
    res.dfs4_addr[0] = '\0';

    res.dfs1_port[0] = '\0';
    res.dfs2_port[0] = '\0';
    res.dfs3_port[0] = '\0';
    res.dfs4_port[0] = '\0';

    res.user_name[0] = '\0';
    res.password[0] = '\0';

    return res;
}

conf_results_dfs_t conf_parsing_default_struct_dfs()
{
    conf_results_dfs_t res;

    res.user1[0] = '\0';
    res.pass1[0] = '\0';
    res.user2[0] = '\0';
    res.pass2[0] = '\0';
    res.user3[0] = '\0';
    res.pass3[0] = '\0';
    res.user4[0] = '\0';
    res.pass4[0] = '\0';
    res.user5[0] = '\0';
    res.pass5[0] = '\0';


    return res;
}


/*******************************************************************************
 * Utilities
 ******************************************************************************/
void conf_parsing_remove_cr_nl(char *buf, int size)
{
    for(int idx = 0; idx < size; idx++)
    {
        if(buf[idx] == '\r')
        {
            buf[idx] = '\0';
        }

        if(buf[idx] == '\n')
        {
            buf[idx] = '\0';
        }
    }
}



/*******************************************************************************
 * Parsing methods
 ******************************************************************************/
conf_results_dfs_t conf_parsing_get_config_dfs(FILE *fd)
{
    conf_results_dfs_t res = conf_parsing_default_struct_dfs();
    char *token = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    /***************************************************************************
     * Parse first user
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct_dfs();
    }
    //  first token is user1
    token = strtok(line, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user1, token);
    // second token is pass1
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.pass1, token);
    
    /***************************************************************************
     * Parse second user
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct_dfs();
    }
    //  first token is user2
    token = strtok(line, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user2, token);
    // second token is pass2
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.pass2, token);   

    /***************************************************************************
     * Parse third user
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct_dfs();
    }
    //  first token is user3
    token = strtok(line, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user3, token);
    // second token is pass3
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.pass3, token);

    /***************************************************************************
     * Parse fourth user
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct_dfs();
    }
    //  first token is user4
    token = strtok(line, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user4, token);
    // second token is pass4
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.pass4, token);

    /***************************************************************************
     * Parse fifth user
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct_dfs();
    }
    //  first token is user5
    token = strtok(line, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user5, token);
    // second token is pass5
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct_dfs();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.pass5, token);
    return res;
}

conf_results_t conf_parsing_get_config_dfc(FILE *fd)
{
    conf_results_t res = conf_parsing_default_struct();
    char *token = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    /***************************************************************************
     * Parse DFS1
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    //  first token is 'Server'
    token = strtok(line, " ");
    // second token is "DFS1"
    token = strtok(NULL, " ");
    // third token is "xxx.xxx.xxx.xxx:######"
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // split third token and get IP
    token = strtok(token, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs1_addr, token);
    // now get port#
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs1_port, token);
    
    /***************************************************************************
     * Parse DFS2
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    //  first token is 'Server'
    token = strtok(line, " ");
    // second token is "DFS2"
    token = strtok(NULL, " ");
    // third token is "xxx.xxx.xxx.xxx:######"
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // split third token and get IP
    token = strtok(token, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs2_addr, token);
    // now get port#
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs2_port, token);

    /***************************************************************************
     * Parse DFS3
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    //  first token is 'Server'
    token = strtok(line, " ");
    // second token is "DFS3"
    token = strtok(NULL, " ");
    // third token is "xxx.xxx.xxx.xxx:######"
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // split third token and get IP
    token = strtok(token, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs3_addr, token);
    // now get port#
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs3_port, token);

    /***************************************************************************
     * Parse DFS4
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    //  first token is 'Server'
    token = strtok(line, " ");
    // second token is "DFS4"
    token = strtok(NULL, " ");
    // third token is "xxx.xxx.xxx.xxx:######"
    token = strtok(NULL, " ");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // split third token and get IP
    token = strtok(token, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs4_addr, token);
    // now get port#
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.dfs4_port, token);

    /***************************************************************************
     * Parse username
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    // split token and get username
    token = strtok(line, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // now get username
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.user_name, token);

    /***************************************************************************
     * Parse password
     **************************************************************************/
    read = getline(&line, &len, fd);
    if(read == -1)
    {
        return conf_parsing_default_struct();
    }
    // split token and get password
    token = strtok(line, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    // now get password
    token = strtok(NULL, ":");
    if(token == NULL)
    {
        return conf_parsing_default_struct();
    }
    conf_parsing_remove_cr_nl(token, strlen(token));
    strcpy(res.password, token);

    return res;
}


/*******************************************************************************
 * Print parsing results.
 ******************************************************************************/
void conf_parsing_print_struct(conf_results_t conf)
{
    printf("dsf1_addr: %s\r\n", conf.dfs1_addr);
    printf("dfs1_port: %s\r\n", conf.dfs1_port);
    printf("dsf2_addr: %s\r\n", conf.dfs2_addr);
    printf("dfs2_port: %s\r\n", conf.dfs2_port);
    printf("dsf3_addr: %s\r\n", conf.dfs3_addr);
    printf("dfs3_port: %s\r\n", conf.dfs3_port);
    printf("dsf4_addr: %s\r\n", conf.dfs4_addr);
    printf("dfs4_port: %s\r\n", conf.dfs4_port);
    printf("user_name: %s\r\n", conf.user_name);
    printf("password: %s\r\n", conf.password);
}

void conf_parsing_print_struct_dfs(conf_results_dfs_t conf)
{
    printf("user1: %s\r\n", conf.user1);
    printf("pass1: %s\r\n", conf.pass1);
    printf("user2: %s\r\n", conf.user2);
    printf("pass2: %s\r\n", conf.pass2);
    printf("user3: %s\r\n", conf.user3);
    printf("pass3: %s\r\n", conf.pass3);
    printf("user4: %s\r\n", conf.user4);
    printf("pass4: %s\r\n", conf.pass4);
    printf("user5: %s\r\n", conf.user5);
    printf("pass5: %s\r\n", conf.pass5);

}
