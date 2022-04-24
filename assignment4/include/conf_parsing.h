#ifndef INC_CONF_PARSING_
#define INC_CONF_PARSING_

#include <stdio.h>

#define MAX_IPv4_STR (16)
#define MAX_IPv4_PRT (6)
#define MAX_USR_NAME (11)
#define MAX_USR_PASS (21)
#define MAX_USERS    (5)

typedef struct {
    char dfs1_addr[MAX_IPv4_STR]; 
    char dfs1_port[MAX_IPv4_PRT];
    char dfs2_addr[MAX_IPv4_STR]; 
    char dfs2_port[MAX_IPv4_PRT];
    char dfs3_addr[MAX_IPv4_STR]; 
    char dfs3_port[MAX_IPv4_PRT];
    char dfs4_addr[MAX_IPv4_STR]; 
    char dfs4_port[MAX_IPv4_PRT];
    char user_name[MAX_USR_NAME];
    char password[MAX_USR_PASS];
} conf_results_t;

typedef struct {
    char user1[MAX_USR_NAME];
    char pass1[MAX_USR_PASS];
    char user2[MAX_USR_NAME];
    char pass2[MAX_USR_PASS];
    char user3[MAX_USR_NAME];
    char pass3[MAX_USR_PASS];
    char user4[MAX_USR_NAME];
    char pass4[MAX_USR_PASS];
    char user5[MAX_USR_NAME];
    char pass5[MAX_USR_PASS];
} conf_results_dfs_t;

conf_results_t conf_parsing_get_config_dfc(FILE *fd);
void conf_parsing_print_struct(conf_results_t conf);
void conf_parsing_remove_cr_nl(char *buf, int size);

conf_results_dfs_t conf_parsing_get_config_dfs(FILE *fd);
void conf_parsing_print_struct_dfs(conf_results_dfs_t conf);
#endif /*INC_CONF_PARSING_*/